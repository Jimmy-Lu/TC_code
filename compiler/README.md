# Directory Structure

- `build_graph`: Main components of the compiler.

- `data_prepare`: Dataset loader for DGL and PyG.

- `models`: GNN models to compile in DGL and PyG.

- `main_dgl.py`/`main_pyg.py`: Entries of compiling GNN models.

# Run

To build a computation graph for your model, add the code below in you main execution file, where the `model` is an instance of your model and `x` is the input feature to the model.
- TODO: we need to provide a standard dataset for parameter extraction.

A sample code can be found in `main_dgl.py` and `main_pyg.py`.
You can directly change the models in those two files to obtain the computation graph.

```bash
    from build_graph import build_graph
    hl_graph = build_graph(model, x)
    staged_code(hl_graph, "xxx")
    hl_graph.save("xxx.pdf")
```
In the above code, function `build_graph` will return an object representing your computation graph, while function `staged_code` will split the graph into different layers and stages and will finally generate assembly code to a `.model` file.  

Inside the function `build_graph`, there are four sub-functions (`opt_graph`, `gen_tiling_graph`, `split_layer`, and `split_stage`) that transform the original computation graph to a computational graph satisfying our need step by step.
- `opt_graph`: remove the redundant or useless ops.

- `gen_tiling_graph`: mark node type through GOPs.

- `split_layer`: split layers.

- `split_stage`: split ops to scatter, gather, apply stages

After function `build_graph`, function `staged_code` generate the code sequence of the staged computational graph.

# Errors

If `torch.onnx` throws a version error, go to `torch/onnx/symbolic_helper.py` and change the default `opset_version` manually.
