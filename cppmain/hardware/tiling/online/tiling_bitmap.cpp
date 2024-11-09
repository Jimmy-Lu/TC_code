//
// Created by Flowerbeach on 2021/3/28.
//

#include "tiling_bitmap.h"
#include "../../memory/mem_top.h"

namespace tiling {
    TilingUnitBitmap::TilingUnitBitmap(MemoryAccessHandler *mah, TileHubTU *th)
            : TilingUnit(mah, th) {
        assert (Config::name_arch == EArch::tu);
        assert (Config::name_tu == ETU::bitmap);
        name = "TU_BITMAP";
        
        b_ids.resize(BitmapMultiple::num_levels);
        b_coos.resize(BitmapMultiple::num_levels);
        b_sizes.resize(BitmapMultiple::num_levels);
        b_buffers.resize(BitmapMultiple::num_levels);
        scan_lens.resize(BitmapMultiple::num_levels);
        b_sizes[1] = Config::bitmap_sizes[1];
        b_sizes[0].first = Config::bitmap_sizes[0].first
                           * Config::bitmap_sizes[1].first;
        b_sizes[0].second = Config::bitmap_sizes[0].second
                            * Config::bitmap_sizes[1].second;
        for (int l = 0; l < BitmapMultiple::num_levels; l++)
            b_ids[l] = {-1, -1};
        b_coos[1].first = 0;
        scan_lens = {4, 16};
        
    }
    
    void TilingUnitBitmap::scan() {
        // level1 submodule
        if (scan_l0_state == 0) {
            begin_level0();
        } else if (scan_l0_state == 1) {
            load_bitmap(0);
        } else if (scan_l0_state == 2) {
            scan_level0();
        } else if (scan_l0_state == 3) {
            wait_level1_to_finish();
        } else if (scan_l0_state == 4) {
            // do nothing, but wait memory
        } else if (scan_l0_state == 5) {
            move_to_next_src();
        } else throw runtime_error("scan_l0_state");
        
        // level2 submodule
        if (scan_l1_state == 0) {
            wait_level0_to_assign();
        } else if (scan_l1_state == 1) {
            scan_level1();
        } else if (scan_l1_state == 2) {
            // do nothing
        } else if (scan_l1_state == 3) {
            load_bitmap(1);
        } else if (scan_l1_state == 4) {
            // do nothing, but wait memory
        } else throw runtime_error("scan_l1_state");
        
    }
    
    void TilingUnitBitmap::issue_to_mah(TypeData type, tag_t tag, callback_t cb) {
        auto req = new RequestLSU();
        req->type_data = type;
        if (type == TypeData::bitmap) {
            req->bitmap = &(b_buffers[tag.at("level")]);
            req->type_req = TypeReqOp::READ;
        } else throw runtime_error("tiling issue_to_mah.\n");
        req->cb_args = new callback_args_t(move(tag));
        req->source_device = name;
        req->callback = std::move(cb);
        mah->issue_access(req);
    }
    
    callback_t TilingUnitBitmap::get_mah_callback(int level) {
        return [this, level](callback_args_t &msg_back) {
            num_accesses++;
            if (level == 0) {
                assert(scan_l0_state == 4);
                scan_l0_state = 2;
            } else if (level == 1) {
                assert(scan_l1_state == 4);
                scan_l1_state = 1;
            } else throw runtime_error("get_mah_callback\n");
        };
    }
    void TilingUnitBitmap::load_bitmap(int level) {
        tag_t tag = {{"level", level},
                     {"id_x",  b_ids[level].first},
                     {"id_y",  b_ids[level].second}};
        b_coos[level].second = 0;
        issue_to_mah(TypeData::bitmap, tag, get_mah_callback(level));
        
        if (level == 0) scan_l0_state = 4;
        else if (level == 1) scan_l1_state = 4;
        else throw runtime_error("load_bitmap\n");
    }
    
}

