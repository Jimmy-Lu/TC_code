import os.path as osp


def main(path_target):
    cnt_tiles_global = 0
    cnt_dst = 0  # dst
    cnt_src = 0  # dst
    cnt_e = 0
    
    with open(osp.join(path_target, 'meta.tile'), 'r') as f:
        items = f.readline().strip().split(' ')
        num_e_total = int(items[0])
        num_v_total = int(items[1])
    
    thread_id = 0
    thread_exits = True
    while (thread_exits):
        with open(osp.join(path_target, '{}.check.tile'.format(thread_id)), 'rb') as f:
            num_tiles_local = int(f.readline().strip())
            cnt_tiles_global += num_tiles_local
        for tile_id in range(num_tiles_local):
            with open(osp.join(path_target, '{}.{}.tile'.format(thread_id, tile_id)), 'rb') as f:
                lines = f.readlines()
            fffff = lines[3].strip()
            flag_output = bool(int(fffff))
            if flag_output is True:
                num_dst = int(lines[9].strip())
                cnt_dst += num_dst
            num_src = int(lines[11].strip())
            num_edge = int(lines[13].strip())
            cnt_src += num_src
            cnt_e += num_edge
        
        thread_id += 1
        thread_exits = osp.exists(osp.join(path_target, '{}.check.tile'.format(thread_id)))
    
    print('cnt_e   - num_e_total: ', cnt_e, num_e_total)
    print('cnt_dst - num_v_total', cnt_dst, num_v_total)
    print('cnt_tiles', cnt_tiles_global)
    print('cnt_src', cnt_src)
    assert cnt_e == num_e_total
    assert cnt_dst == num_v_total


if __name__ == '__main__':
    path = 'dataset.{}/tiling.{}/{}_{}/'.format(
        'AK', 'Sparse', 512, 1024)
    main(path)
