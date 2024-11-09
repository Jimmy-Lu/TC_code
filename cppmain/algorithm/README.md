## Graph Operation

- GTHR: gather
- SCTR: scatter

### Direction

- F: forward (src2edge / edge2dst)
- B: backward (dst2edge / edge2src)

### Aggregator

- SUM, MAX

## Operation Dimension

### Macro

- NE: number of edges in a tile
- NSRC: number of source vertices in a tile
- NDST: number of destination vertices in a tile

### Operation Type

- V: vector
- S: scalar
- VS: vector&scalar
- SS: scalar&scalar / vector&vector
- VV_W: feature vector & feature vector
- VV_F: feature vector & weight vector
