# -*- coding: UTF-8 -*-
import os
import time
import xlwt

root = os.getcwd()

cell_style_head = xlwt.easyxf("align: vertical center, horizontal center; font: bold true, colour black;")
cell_style_content = xlwt.easyxf("align: vertical center, horizontal center; font: colour black;")

models = ['gat', 'gcn', 'ggnn', 'sage']
tilings = ['sptr', 'hygcn', 'baseline']
datasets = ['ak', 'ad', 'cp', 'hw', 'sl']
archs = ['base', 'tu']

# column idx
cid_model = 0
cid_dataset = 1
cid_tiling = 2
cid_reorder = 3
cid_arch = 4
cid_num_vu = 5
cid_num_mu = 6
cid_num_tu = 7
cid_num_thread = 8
cid_coalesce = 9
cid_vu_balance = 10

cid_latency_ms = 11
cid_num_tile = 12
cid_num_coalesced = 13
cid_energy_mJ = 14
cid_read_dram_MB = 15
cid_write_dram_MB = 16
cid_bandwidth_GB_s = 17
cid_bandwidth_percent = 18
cid_tu_utilization_percent = 19
cid_mu_utilization_percent = 20
cid_vu_utilization_percent = 21

cid_remark = 22

CNT = 0


class ResultStructure(object):
    def __init__(self, filename: str):
        self.filename = filename
        filename = filename.replace('.txt', '')
        items = filename.split('_')
        
        self.num_mu = -1
        self.num_vu = -1
        self.num_tu = -1
        
        self.vu_balance = False
        self.coalesce = False
        
        self.prefix = items[0]
        self.model = items[1]
        self.dataset = items[2]
        self.arch = items[3]
        self.tiling = items[4]
        self.reorder = items[5]
        self.num_thread = int(items[6])
        self.remark = items[7]
        
        print('prefix:', self.prefix, end=', ')
        print('model:', self.model, end=', ')
        print('dataset:', self.dataset, end=', ')
        print('arch:', self.arch, end=', ')
        print('tiling:', self.tiling, end=', ')
        print('reorder:', self.reorder, end=', ')
        print('num_thread:', self.num_thread, end=', ')
        print('remark:', self.remark)
        
        self.latency_ms = None
        self.num_tile = None
        self.dram_read_MB = None
        self.dram_write_MB = None
        self.bandwidth_avg = None
        self.bandwidth_per = None
        self.mu_ut = None
        self.vu_ut = None
        self.tu_ut = None
        self.energy_mJ = None
        self.num_coalesced = 0


def extract_file_content(filename: str, worksheet: xlwt.Worksheet):
    global CNT
    
    print(filename)
    rs = ResultStructure(filename)
    f = open(root + "/" + filename)
    lines = f.readlines()
    for line in lines:
        line = line.strip()
        ####################################################
        if 'vu#' in line and 'mu#' in line and 'thread#' in line:
            arch = line.split('=')[1].split(',')[0].strip()
            num_thread = int(line.split('=')[2].split(',')[0])
            rs.num_vu = int(line.split('=')[3].split(',')[0])
            rs.num_mu = int(line.split('=')[4].split(',')[0])
            rs.num_tu = int(line.split('=')[5].split('.')[0])
            assert (rs.num_thread == num_thread)
            assert (rs.arch == arch)
        ####################################################
        if 'coalesce=' in line and 'vu-balance=' in line:
            rs.coalesce = (line.split('=')[1].split(',')[0] == '1')
            rs.vu_balance = (line.split('=')[2].strip() == '1')
        ####################################################
        if 'dataset=' in line and 'tiling=' in line:
            assert line == 'dataset={}, tiling={}, format=binary.'.format(rs.dataset, rs.tiling)
        ####################################################
        if 'model=' in line:
            assert line == 'model={}.'.format(rs.model)
        ####################################################
        if 'Cycle#: ' in line:
            rs.latency_ms = float(line.split(': ')[1].split(' ms')[0])
            rs.num_tile = int(line.split(': ')[2].split(',')[0])
            rs.num_coalesced = int(line.split(': ')[3].strip())
        if 'Off-chip read:' in line:
            rs.dram_read_MB = float(line.split(' ')[-2])
        if 'Off-chip write:' in line:
            rs.dram_write_MB = float(line.split(' ')[-2])
        if 'Bandwidth avg:' in line:
            rs.bandwidth_avg = float(line.split(' GB/s')[0].split(' ')[-1])
            rs.bandwidth_per = float(line.split(' %')[0].split(' ')[-1])
        if 'MU utilization' in line:
            rs.mu_ut = float(line.split(' ')[-2])
        if 'VU utilization' in line:
            rs.vu_ut = float(line.split(' ')[-2])
        if 'TU utilization' in line:
            rs.tu_ut = float(line.split(' ')[-2])
        if 'Energy Total' in line:
            rs.energy_mJ = float(line.split(' ')[-2])
        ####################################################
    f.close()
    
    ####################################################
    worksheet.write(CNT, cid_arch, rs.arch, cell_style_content)
    worksheet.write(CNT, cid_model, rs.model, cell_style_content)
    worksheet.write(CNT, cid_tiling, rs.tiling, cell_style_content)
    worksheet.write(CNT, cid_dataset, rs.dataset, cell_style_content)
    worksheet.write(CNT, cid_reorder, rs.reorder, cell_style_content)
    worksheet.write(CNT, cid_num_vu, rs.num_vu, cell_style_content)
    worksheet.write(CNT, cid_num_mu, rs.num_mu, cell_style_content)
    worksheet.write(CNT, cid_num_tu, rs.num_tu, cell_style_content)
    worksheet.write(CNT, cid_num_thread, rs.num_thread, cell_style_content)
    worksheet.write(CNT, cid_vu_balance, rs.vu_balance, cell_style_content)
    worksheet.write(CNT, cid_coalesce, rs.coalesce, cell_style_content)
    ####################################################
    worksheet.write(CNT, cid_latency_ms, rs.latency_ms, cell_style_content)
    worksheet.write(CNT, cid_num_tile, rs.num_tile, cell_style_content)
    worksheet.write(CNT, cid_num_coalesced, rs.num_coalesced, cell_style_content)
    worksheet.write(CNT, cid_read_dram_MB, rs.dram_read_MB, cell_style_content)
    worksheet.write(CNT, cid_write_dram_MB, rs.dram_write_MB, cell_style_content)
    worksheet.write(CNT, cid_bandwidth_GB_s, rs.bandwidth_avg, cell_style_content)
    worksheet.write(CNT, cid_bandwidth_percent, rs.bandwidth_per, cell_style_content)
    worksheet.write(CNT, cid_mu_utilization_percent, rs.mu_ut, cell_style_content)
    worksheet.write(CNT, cid_vu_utilization_percent, rs.vu_ut, cell_style_content)
    worksheet.write(CNT, cid_tu_utilization_percent, rs.tu_ut, cell_style_content)
    worksheet.write(CNT, cid_energy_mJ, rs.energy_mJ, cell_style_content)
    ####################################################
    worksheet.write(CNT, cid_remark, rs.remark, cell_style_content)
    CNT += 1


def select_files():
    filenames = os.listdir(root)
    targeted_filenames = []
    
    # remove gpu results
    for filename in filenames:
        if not filename.startswith('e_'):
            continue
        targeted_filenames.append(filename)
    return targeted_filenames


def header(worksheet):
    global CNT
    worksheet.write(CNT, cid_remark, 'Remark', cell_style_head)
    worksheet.write(CNT, cid_arch, 'Arch', cell_style_head)
    worksheet.write(CNT, cid_model, 'Model', cell_style_head)
    worksheet.write(CNT, cid_dataset, 'Dataset', cell_style_head)
    worksheet.write(CNT, cid_tiling, 'Tiling', cell_style_head)
    worksheet.write(CNT, cid_reorder, 'Reorder', cell_style_head)
    worksheet.write(CNT, cid_coalesce, 'Coalesce', cell_style_head)
    worksheet.write(CNT, cid_vu_balance, 'VU-balance', cell_style_head)
    worksheet.write(CNT, cid_latency_ms, 'Latency/ms', cell_style_head)
    worksheet.write(CNT, cid_energy_mJ, 'Energy/mJ', cell_style_head)
    worksheet.write(CNT, cid_read_dram_MB, 'Read/MB', cell_style_head)
    worksheet.write(CNT, cid_write_dram_MB, 'Write/MB', cell_style_head)
    worksheet.write(CNT, cid_bandwidth_GB_s, 'Bandwidth/GB/s', cell_style_head)
    worksheet.write(CNT, cid_bandwidth_percent, 'Bandwidth/%', cell_style_head)
    worksheet.write(CNT, cid_tu_utilization_percent, 'tu utilization/%', cell_style_head)
    worksheet.write(CNT, cid_mu_utilization_percent, 'mu utilization/%', cell_style_head)
    worksheet.write(CNT, cid_vu_utilization_percent, 'vu utilization/%', cell_style_head)
    worksheet.write(CNT, cid_num_coalesced, 'Coalesced#', cell_style_head)
    worksheet.write(CNT, cid_num_thread, 'Thread#', cell_style_head)
    worksheet.write(CNT, cid_num_tile, 'Tile#', cell_style_head)
    worksheet.write(CNT, cid_num_tu, 'tu#', cell_style_head)
    worksheet.write(CNT, cid_num_vu, 'vu#', cell_style_head)
    worksheet.write(CNT, cid_num_mu, 'mu#', cell_style_head)
    CNT += 1


def main():
    workbook = xlwt.Workbook(encoding='ascii')
    worksheet = workbook.add_sheet(str(int(time.time())), cell_overwrite_ok=True)
    header(worksheet)
    
    filenames = select_files()
    for filename in filenames:
        extract_file_content(filename, worksheet)
    workbook.save('result.xls')


if __name__ == '__main__':
    main()
