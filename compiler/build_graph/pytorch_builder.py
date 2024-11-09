"""
HiddenLayer

PyTorch graph importer.
 
Written by Waleed Abdulla
Licensed under the MIT License
"""

from __future__ import absolute_import, division, print_function
import re
from .graph import Graph, Node
from . import transforms as ht
import torch

# PyTorch Graph Transforms
FRAMEWORK_TRANSFORMS = [
    # Hide onnx: prefix
    ht.Rename(op=r"onnx::(.*)", to=r"\1"),
    # ONNX uses Gemm for linear layers (stands for General Matrix Multiplication).
    # It's an odd name that noone recognizes. Rename it. 
    ht.Rename(op=r"Gemm", to=r"Linear"),
    # PyTorch layers that don't have an ONNX counterpart
    ht.Rename(op=r"aten::max\_pool2d\_with\_indices", to="MaxPool"),
    # Shorten op name
    ht.Rename(op=r"BatchNormalization", to="BatchNorm"),
]

REDUNDANT_OPS = [
    "Reshape", "Unsqueeze", "Flatten", "Transpose"
]


def dump_pytorch_graph(graph):
    """List all the nodes in a PyTorch graph."""
    f = "{:25} {:40}   {} -> {}"
    print(f.format("kind", "scopeName", "inputs", "outputs"))
    for node in graph.nodes():
        print(f.format(node.kind(), node.scopeName(),
                       [i.unique() for i in node.inputs()],
                       [i.unique() for i in node.outputs()]
                       ))


def pytorch_id(node):
    """Returns a unique ID for a node."""
    # After ONNX simplification, the scopeName is not unique anymore
    # so append node outputs to guarantee uniqueness
    return node.scopeName() + "/outputs/" + "/".join(["{}".format(o.unique()) for o in node.outputs()])


def pytorch_id_input(node):
    # return a unique ID for a input node
    return "/inputs/" + str(node.unique())


def get_shape(torch_node):
    """Return the output shape of the given Pytorch node."""
    # Extract node output shape from the node string representation
    # This is a hack because there doesn't seem to be an official way to do it.
    # See my quesiton in the PyTorch forum:
    # https://discuss.pytorch.org/t/node-output-shape-from-trace-graph/24351/2
    # TODO: find a better way to extract output shape
    # TODO: Assuming the node has one output. Update if we encounter a multi-output node.
    # m = re.match(r".*Float\(([\d\s\,]+)\).*", str(next(torch_node.outputs())))
    # if m:
    #     shape = m.group(1)
    #     shape = shape.split(",")
    #     shape = tuple(map(int, shape))
    # else:
    #     shape = None
    shape = torch_node.output().type().sizes()
    return shape


def get_shape_input(node, count):
    # return the shape of input node, not an official way
    input = str(node.node()).split("%")[count]
    m = re.match(r".*Float\((.*), strides.*", input)
    if m:
        shape = m.group(1).split(",")
        shape = list(map(int, shape))
    else:
        shape = None
    return shape


def get_op(torch_node):
    # get the op name of a node
    m1 = re.match(r".*= onnx::(.*)\[(a|p|v).*\]\(.*", str(next(torch_node.outputs())))  # op like ReduceSum
    m2 = re.match(r".*= onnx::(.*)\(.*\).*", str(next(torch_node.outputs())))  # op like Mul
    m3 = re.match(r".*= \^(.*)\((\<.*\>)(.*)\)\(.*", str(next(torch_node.outputs())))  # op like GSpMM
    m4 = re.match(r".*\^(.*)\(\).*", str(next(torch_node.outputs())))  # ops in pyg
    m5 = re.match(r".*= \^(.*)\((.*)\)\(.*", str(next(torch_node.outputs())))  #remaining ops like SegmentReduce
    if m1:
        op = m1.group(1)
    elif m2:
        op = m2.group(1)
    elif m3:
        op = m3.group(1) + m3.group(3)
        op = op.split(", ")
        op = tuple(map(str, op))
    elif m4:
        op = m4.group(1)
    elif m5:
        op = m5.group(1) +"_" + m5.group(2)
    else:
        op = None
    if op[0] == 'EdgeSoftmax':
        op = op[0]
    else:
        op = op
    return op


# remove the redundant or useless ops
# todo opt the redundant codes below
def opt_graph(hl_graph):
    id_set = hl_graph.id_list
    start = []
    end = []
    for i in range(len(id_set)):
        id = id_set[i]
        if hl_graph.nodes[id].op == 'Gather_max' or hl_graph.nodes[id].op == 'Gather_min':
            start.append(i + 1)
        elif hl_graph.nodes[id].op == "Where":
            end.append(i + 1)
        elif hl_graph.nodes[id].op in REDUNDANT_OPS:
            start.append(i)
            end.append(i + 1)

    id_set1 = id_set[:]
    for i in range(len(start)):
        _start = start[i]
        _end = end[i]
        # connect the nodes before and after the deleted node
        for _id in id_set:
            if hl_graph.nodes[_id].input is not None and \
                    hl_graph.nodes[id_set1[_end - 1]].output[0] in hl_graph.nodes[_id].input:
                hl_graph.nodes[_id].input.remove(hl_graph.nodes[id_set1[_end - 1]].output[0])
                for input in hl_graph.nodes[id_set1[_start]].input:
                    hl_graph.nodes[_id].input.append(input)
                # delete transpose op and tanspose the input of transpose op
                if hl_graph.nodes[id_set1[_start]].op == 'Transpose':
                    node_pre = hl_graph.nodes[id_set1[_start]].input[0]
                    shape = hl_graph.nodes[node_pre].output_shape
                    shape = shape[::-1]
                    hl_graph.nodes[node_pre].output_shape = shape
                    hl_graph.nodes[node_pre].op = shape

        # delete the redundant or useless node(like reshape or where)
        for j in range(_start, _end):
            id_ = id_set1[j]
            hl_graph.remove(hl_graph.nodes[id_])

    # delete Constant
    # todo find a better way to delete
    for id in id_set:
        if hl_graph.nodes[id].op == "Constant":
            hl_graph.remove(hl_graph.nodes[id])
            for id_in in id_set:
                if hl_graph.nodes[id_in].input is not None and id in hl_graph.nodes[id_in].input:
                    hl_graph.nodes[id_in].input.remove(id)


def node_type_back(hl_graph, node, attr):
    if node.input is not None:
        for input in node.input:
            if hl_graph.nodes[input].op[:6] == "Gather" or hl_graph.nodes[input].op[:7] == "Scatter":
                break
            elif hl_graph.nodes[input].symbol == ['W']:
                continue
            else:
                hl_graph.nodes[input].symbol.append(attr)
                node_type_back(hl_graph, hl_graph.nodes[input], attr)
    else:
        node.symbol.append(attr)


def node_type_forward(hl_graph, node, attr):
    id_set = hl_graph.id_list
    count = [i for (i, j) in enumerate(id_set) if j == node.id][0]
    id_set = id_set[count + 1:]
    for id in id_set:
        if hl_graph.nodes[id].input is not None:
            if hl_graph.nodes[id].op[:6] == "Gather" or hl_graph.nodes[id].op[:7] == "Scatter":
                flag = 1
            else:
                flag = 0
            if node.id in hl_graph.nodes[id].input and flag == 0:
                hl_graph.nodes[id].symbol.append(attr)
                node_type_forward(hl_graph, hl_graph.nodes[id], attr)
            elif flag == 1:
                break


# mark node type through GOPs
def gen_tiling_graph(hl_graph):
    id_set = hl_graph.id_list
    scat_src_id = []
    scat_dst_id = []
    gather_id = []
    for id in id_set:
        if hl_graph.nodes[id].op == "Scatter_outedge":
            hl_graph.nodes[id].symbol.append('Edge')
            scat_src_id.append(id)
        elif hl_graph.nodes[id].op == "Scatter_inedge":
            hl_graph.nodes[id].symbol.append('Edge')
            scat_dst_id.append(id)
        elif hl_graph.nodes[id].op[:6] == "Gather":
            hl_graph.nodes[id].symbol.append('Edge')
            gather_id.append(id)
        else:
            continue

    # add node type
    for id_src in scat_src_id:
        node_type_back(hl_graph, hl_graph.nodes[id_src], 'Src')
        node_type_forward(hl_graph, hl_graph.nodes[id_src], 'Edge')
    for id_dst in scat_dst_id:
        node_type_back(hl_graph, hl_graph.nodes[id_dst], 'Dst')
        node_type_forward(hl_graph, hl_graph.nodes[id_dst], 'Edge')
    for id_e in gather_id:
        node_type_back(hl_graph, hl_graph.nodes[id_e], 'Edge')
        node_type_forward(hl_graph, hl_graph.nodes[id_e], 'Dst')

    # get set
    for id in id_set:
        hl_graph.nodes[id].symbol = list(set(hl_graph.nodes[id].symbol))


# build a layer from the last node
def build_layer(hl_graph, split_id, node, layers, layer):
    id_q = []
    id_q.append(node.id)
    visited = []
    while len(id_q) > 0:
        id = id_q[0]
        if hl_graph.nodes[id].input is not None:
            for input in hl_graph.nodes[id].input:
                if input not in visited and input not in split_id:
                    id_q.append(input)
                    visited.append(input)
        layer.append(id)
        id_q.remove(id)
    layers.append(layer)


# split layers
def split_layer(hl_graph):
    id_set = hl_graph.id_list
    layer_id = []
    split_id = []
    src_count = 0
    dst_count = 0
    edge_count = 0
    # count node type orders and get layers split ids
    for id in id_set:
        if 'Src' in hl_graph.nodes[id].symbol:
            hl_graph.nodes[id].symbol_order[0] = src_count
            src_count = src_count + 1
        if 'Dst' in hl_graph.nodes[id].symbol:
            hl_graph.nodes[id].symbol_order[1] = dst_count
            dst_count = dst_count + 1
        elif 'Edge' in hl_graph.nodes[id].symbol:
            hl_graph.nodes[id].symbol_order[2] = edge_count
            edge_count = edge_count + 1
        if hl_graph.nodes[id].op[:6] == "Gather":
            hl_graph.nodes[id].symbol_order[1] = dst_count
            dst_count = dst_count + 1
        if 'Src' in hl_graph.nodes[id].symbol and 'Dst' in hl_graph.nodes[id].symbol:
            if hl_graph.nodes[id].input is not None:
                for input in hl_graph.nodes[id].input:
                    if input in layer_id:
                        layer_id.remove(input)
            layer_id.append(id)
            split_id.append(id)
        else:
            continue
    # print(layer_id)

    # build layers
    id_set_r = id_set[::-1]
    layers = hl_graph.layers
    layer = []
    build_layer(hl_graph, split_id, hl_graph.nodes[id_set_r[0]], layers, layer)
    for id in layer_id:
        layer = []
        build_layer(hl_graph, layer_id, hl_graph.nodes[id], layers, layer)
    # print(layers)
    return layers


# split ops to scatter, gather, apply stages
def split_stage(hl_graph, layer):
    apply_stage = hl_graph.apply_stage
    gather_stage = hl_graph.gather_stage
    scatter_stage = hl_graph.scatter_stage
    id_apply_start = []
    id_1 = layer[0]
    id_apply_start.append(id_1)
    visited = []
    id_gather_start = []
    id_scatter_start = []
    # apply
    while len(id_apply_start) > 0:
        id = id_apply_start.pop()
        if 'Src' in hl_graph.nodes[id].symbol or 'Dst' in hl_graph.nodes[id].symbol:
            apply_stage.append(id)
            visited.append(id)
        # start id of gather stage
        elif hl_graph.nodes[id].symbol == ['Edge']:
            id_gather_start.append(id)
        if hl_graph.nodes[id].input is not None and id not in id_gather_start:
            for input in hl_graph.nodes[id].input:
                if input not in visited and input in layer:
                    id_apply_start.append(input)
    # gather
    while len(id_gather_start) > 0:
        id = id_gather_start.pop()
        if hl_graph.nodes[id].symbol == ['Edge'] or hl_graph.nodes[id].symbol == ['Src']:
            gather_stage.append(id)
            visited.append(id)
        elif 'Dst' in hl_graph.nodes[id].symbol:
            id_scatter_start.append(id)
        if hl_graph.nodes[id].input is not None and id not in id_scatter_start:
            for input in hl_graph.nodes[id].input:
                if input not in visited and input in layer:
                    id_gather_start.append(input)
    # scatter
    while len(id_scatter_start) > 0:
        id = id_scatter_start.pop()
        if hl_graph.nodes[id].symbol == ['Dst']:
            scatter_stage.append(id)
            visited.append(id)
        # elif 'Src' in hl_graph.nodes[id].symbol or hl_graph.nodes[id].symbol == ['Edge']:
        #     continue
        if hl_graph.nodes[id].input is not None:
            for input in hl_graph.nodes[id].input:
                if input not in visited and input in layer:
                    id_scatter_start.append(input)

    return apply_stage, gather_stage, scatter_stage


def import_graph(hl_graph, model, args, input_names=None, verbose=False):
    # Run the Pytorch graph to get a trace and generate a graph from it
    trace, out = torch.jit._get_trace_graph(model, args)
    torch_graph = torch.onnx._optimize_trace(trace, torch.onnx.OperatorExportTypes.ONNX)

    # Dump list of nodes (DEBUG only)
    if verbose:
        dump_pytorch_graph(torch_graph)

    # add inputs to graph
    count = 1
    for torch_node in torch_graph.inputs():
        # input name
        # for input name, we define its name with its shape
        if count == 1:
            input_name = "input0"
        else:
            input_name = str(get_shape_input(torch_node, count))
        # Parameters
        params = None
        outputs = [torch_node.unique()]
        # Get output shape
        shape = get_shape_input(torch_node, count)

        # Add HL node
        hl_node = Node(uid=outputs[0], name=None, op=input_name, input=None, output=outputs,
                       output_shape=shape, params=params)
        hl_graph.add_node(hl_node)
        hl_graph.add_input_symbol(hl_node, count)
        count = count + 1

    # Loop through nodes and build HL graph
    for torch_node in torch_graph.nodes():
        # Op
        # op = torch_node.kind()
        op = get_op(torch_node)
        # Parameters
        params = {k: torch_node[k] for k in torch_node.attributeNames()}
        # Inputs/outputs
        inputs = [i.unique() for i in torch_node.inputs()]
        outputs = [o.unique() for o in torch_node.outputs()]
        # Get output shape
        shape = get_shape(torch_node)
        if op[0] == "GSDDMM":
            # transform GSDDMM to two scatter and a user-defined op
            output_inter1 = [str(outputs[0]) + "_1"]
            output_inter2 = [str(outputs[0]) + "_2"]
            input_inter1 = [output_inter1[0], output_inter2[0]]
            input_inter2 = [output_inter1[0], inputs[0]]
            id1 = output_inter1[0]
            id2 = output_inter2[0]
            id3 = outputs[0]
            # to be extended except (u,v),(e,v)
            if op[2] == 'u' and op[3] == 'v':
                hl_node1 = Node(uid=id1, name=None, op="Scatter_outedge", input=[inputs[0]], output=output_inter1,
                                output_shape=shape, params=params)
                hl_node2 = Node(uid=id2, name=None, op="Scatter_inedge", input=[inputs[1]], output=output_inter2,
                                output_shape=shape, params=params)
                hl_node3 = Node(uid=id3, name=None, op=str(op[1]), input=input_inter1, output=outputs,
                                output_shape=shape, params=params)
                hl_graph.add_node(hl_node1)
                hl_graph.add_node(hl_node2)
                hl_graph.add_node(hl_node3)
            elif op[2] == 'e' and op[3] == 'v':
                hl_node1 = Node(uid=id1, name=None, op="Scatter_inedge", input=[inputs[1]], output=output_inter1,
                                output_shape=shape, params=params)
                hl_node3 = Node(uid=id3, name=None, op=str(op[1]), input=input_inter2, output=outputs,
                                output_shape=shape, params=params)
                hl_graph.add_node(hl_node1)
                hl_graph.add_node(hl_node3)

        elif op[0] == "GSpMM":
            # transform GSpMM to user-defined ops
            output_inter = [str(outputs[0]) + "_1"]
            input_inter = [output_inter[0]]
            id1 = str(outputs[0]) + "_1"
            id2 = outputs[0]
            if op[1] == 'copy_lhs':
                op1 = "Scatter_outedge"
            elif op[1] == 'copy_rhs':
                op1 = "Scatter_inedge"
            else:
                op1 = str(op[1])
            if len(inputs) == 1:
                if hl_graph.nodes[inputs[0]].attr[0] == 'E':
                    hl_node2 = Node(uid=id2, name=None, op="Gather_" + str(op[2]), input=inputs, output=outputs,
                                    output_shape=shape, params=params)
                else:
                    hl_node1 = Node(uid=id1, name=None, op=op1, input=inputs, output=output_inter,
                                    output_shape=hl_graph.nodes[inputs[0]].output_shape, params=params)
                    hl_node2 = Node(uid=id2, name=None, op="Gather_" + str(op[2]), input=input_inter, output=outputs,
                                    output_shape=shape, params=params)
                    hl_graph.add_node(hl_node1)
            else:
                hl_node1 = Node(uid=id1, name=None, op=op1, input=inputs, output=output_inter,
                                output_shape=None, params=params)
                hl_node2 = Node(uid=id2, name=None, op="Gather_" + str(op[2]), input=input_inter, output=outputs,
                                output_shape=shape, params=params)
                hl_graph.add_node(hl_node1)
                new_inputs = inputs
                count = 2
                flag = 0
                for input in inputs:
                    if hl_graph.nodes[input].attr[0] == "V":
                        flag = 1
                        id = str(outputs[0]) + "_" + str(count)
                        output_inter_ = [id]
                        new_inputs.remove(input)
                        new_inputs.append(output_inter_[0])
                        hl_node = Node(uid=id, name=None, op="Scatter_outedge", input=[input], output=output_inter_,
                                       output_shape=None, params=params)
                        hl_graph.add_node(hl_node)
                if flag == 1:
                    hl_node1.input = new_inputs

            hl_graph.add_node(hl_node2)
        else:
            # Add HL node
            hl_node = Node(uid=outputs[0], name=None, op=str(op), input=inputs, output=outputs,
                           output_shape=shape, params=params)
            hl_graph.add_node(hl_node)

    opt_graph(hl_graph)
    gen_tiling_graph(hl_graph)
    split_layer(hl_graph)
    for layer in hl_graph.layers:
        split_stage(hl_graph, layer)

    # print(hl_graph.apply_stage)
    # print(hl_graph.gather_stage)
    # print(hl_graph.scatter_stage)

    # add edges to build whole graph
    for id_in in hl_graph.id_list:
        # print(hl_graph.nodes[id_in].id, hl_graph.nodes[id_in].op, hl_graph.nodes[id_in].symbol)
        for id_out in hl_graph.id_list:
            if hl_graph.nodes[id_out].input is not None and \
                    set(hl_graph.nodes[id_out].input) & set(hl_graph.nodes[id_in].output):
                if (id_in, id_out, hl_graph.nodes[id_out].output_shape) not in hl_graph.edges:
                    hl_graph.add_edge_by_id(id_in, id_out, hl_graph.nodes[id_in].output_shape)
    return hl_graph
