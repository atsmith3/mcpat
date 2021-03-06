/*****************************************************************************
 *                                McPAT
 *                      SOFTWARE LICENSE AGREEMENT
 *            Copyright 2012 Hewlett-Packard Development Company, L.P.
 *                          All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.”
 *
 ***************************************************************************/

#include "sharedcache.h"

#include "XML_Parse.h"
#include "arbiter.h"
#include "array.h"
#include "basic_circuit.h"
#include "const.h"
#include "io.h"
#include "parameter.h"

#include <algorithm>
#include <assert.h>
#include <cmath>
#include <iostream>
#include <string.h>

SharedCache::SharedCache() {
  XML = nullptr;
  long_channel = false;
  power_gating = false;
  init_params = false;
  init_stats = false;
  set_area = false;
  cacheL = L2;
  ithCache = 0;
  dir_overhead = 0.0;
  scktRatio = 0.0;
  executionTime = 0.0;

  device_t = Core_device;
  core_t = OOO;

  debug = false;
  is_default = false;

  size = 0.0;
  line = 0.0;
  assoc = 0.0;
  banks = 0.0;
}

void SharedCache::set_params(const ParseXML *XML,
                             const int ithCache,
                             InputParameter *interface_ip_,
                             const enum cache_level cacheL_) {
  int idx = 0;
  int tag = 0;
  int data = 0;
  this->cacheL = cacheL_;
  this->interface_ip = *interface_ip_;
  this->ithCache = ithCache;

  if (cacheL == L2 && XML->sys.Private_L2) {
    device_t = Core_device;
    core_t = (enum Core_type)XML->sys.core[ithCache].machine_type;
  } else {
    device_t = LLC_device;
    core_t = Inorder;
  }

  switch (cacheL) {
    case L2: {
      cachep.set_params_l2_cache(XML, ithCache);
      interface_ip.data_arr_ram_cell_tech_type =
          XML->sys.L2[ithCache].device_type; // long channel device LSTP
      interface_ip.data_arr_peri_global_tech_type =
          XML->sys.L2[ithCache].device_type;
      interface_ip.tag_arr_ram_cell_tech_type =
          XML->sys.L2[ithCache].device_type;
      interface_ip.tag_arr_peri_global_tech_type =
          XML->sys.L2[ithCache].device_type;
      if (XML->sys.Private_L2 && XML->sys.core[ithCache].vdd > 0) {
        interface_ip.specific_hp_vdd = true;
        interface_ip.specific_lop_vdd = true;
        interface_ip.specific_lstp_vdd = true;
        interface_ip.hp_Vdd = XML->sys.core[ithCache].vdd;
        interface_ip.lop_Vdd = XML->sys.core[ithCache].vdd;
        interface_ip.lstp_Vdd = XML->sys.core[ithCache].vdd;
      }
      if (XML->sys.Private_L2 &&
          XML->sys.core[ithCache].power_gating_vcc > -1) {
        interface_ip.specific_vcc_min = true;
        interface_ip.user_defined_vcc_min =
            XML->sys.core[ithCache].power_gating_vcc;
      }
      if (!XML->sys.Private_L2 && XML->sys.L2[ithCache].vdd > 0) {
        interface_ip.specific_hp_vdd = true;
        interface_ip.specific_lop_vdd = true;
        interface_ip.specific_lstp_vdd = true;
        interface_ip.hp_Vdd = XML->sys.L2[ithCache].vdd;
        interface_ip.lop_Vdd = XML->sys.L2[ithCache].vdd;
        interface_ip.lstp_Vdd = XML->sys.L2[ithCache].vdd;
      }
      if (!XML->sys.Private_L2 && XML->sys.L2[ithCache].power_gating_vcc > -1) {
        interface_ip.specific_vcc_min = true;
        interface_ip.user_defined_vcc_min =
            XML->sys.L2[ithCache].power_gating_vcc;
      }
      break;
    }
    case L3: {
      cachep.set_params_l3_cache(XML, ithCache);
      interface_ip.data_arr_ram_cell_tech_type =
          XML->sys.L3[ithCache].device_type; // long channel device LSTP
      interface_ip.data_arr_peri_global_tech_type =
          XML->sys.L3[ithCache].device_type;
      interface_ip.tag_arr_ram_cell_tech_type =
          XML->sys.L3[ithCache].device_type;
      interface_ip.tag_arr_peri_global_tech_type =
          XML->sys.L3[ithCache].device_type;
      if (XML->sys.L3[ithCache].vdd > 0) {
        interface_ip.specific_hp_vdd = true;
        interface_ip.specific_lop_vdd = true;
        interface_ip.specific_lstp_vdd = true;
        interface_ip.hp_Vdd = XML->sys.L3[ithCache].vdd;
        interface_ip.lop_Vdd = XML->sys.L3[ithCache].vdd;
        interface_ip.lstp_Vdd = XML->sys.L3[ithCache].vdd;
      }
      if (XML->sys.L3[ithCache].power_gating_vcc > -1) {
        interface_ip.specific_vcc_min = true;
        interface_ip.user_defined_vcc_min =
            XML->sys.L3[ithCache].power_gating_vcc;
      }
      break;
    }
    case L1Directory: {
      cachep.set_params_l1_directory(XML, ithCache);
      interface_ip.data_arr_ram_cell_tech_type =
          XML->sys.L1Directory[ithCache]
              .device_type; // long channel device LSTP
      interface_ip.data_arr_peri_global_tech_type =
          XML->sys.L1Directory[ithCache].device_type;
      interface_ip.tag_arr_ram_cell_tech_type =
          XML->sys.L1Directory[ithCache].device_type;
      interface_ip.tag_arr_peri_global_tech_type =
          XML->sys.L1Directory[ithCache].device_type;
      if (XML->sys.L1Directory[ithCache].vdd > 0) {
        interface_ip.specific_hp_vdd = true;
        interface_ip.specific_lop_vdd = true;
        interface_ip.specific_lstp_vdd = true;
        interface_ip.hp_Vdd = XML->sys.L1Directory[ithCache].vdd;
        interface_ip.lop_Vdd = XML->sys.L1Directory[ithCache].vdd;
        interface_ip.lstp_Vdd = XML->sys.L1Directory[ithCache].vdd;
      }
      if (XML->sys.L1Directory[ithCache].power_gating_vcc > -1) {
        interface_ip.specific_vcc_min = true;
        interface_ip.user_defined_vcc_min =
            XML->sys.L1Directory[ithCache].power_gating_vcc;
      }
      break;
    }
    case L2Directory: {
      cachep.set_params_l2_directory(XML, ithCache);
      interface_ip.data_arr_ram_cell_tech_type =
          XML->sys.L2Directory[ithCache]
              .device_type; // long channel device LSTP
      interface_ip.data_arr_peri_global_tech_type =
          XML->sys.L2Directory[ithCache].device_type;
      interface_ip.tag_arr_ram_cell_tech_type =
          XML->sys.L2Directory[ithCache].device_type;
      interface_ip.tag_arr_peri_global_tech_type =
          XML->sys.L2Directory[ithCache].device_type;
      if (XML->sys.L2Directory[ithCache].vdd > 0) {
        interface_ip.specific_hp_vdd = true;
        interface_ip.specific_lop_vdd = true;
        interface_ip.specific_lstp_vdd = true;
        interface_ip.hp_Vdd = XML->sys.L2Directory[ithCache].vdd;
        interface_ip.lop_Vdd = XML->sys.L2Directory[ithCache].vdd;
        interface_ip.lstp_Vdd = XML->sys.L2Directory[ithCache].vdd;
      }
      if (XML->sys.L2Directory[ithCache].power_gating_vcc > -1) {
        interface_ip.specific_vcc_min = true;
        interface_ip.user_defined_vcc_min =
            XML->sys.L2Directory[ithCache].power_gating_vcc;
      }
      break;
    }
    default: {
      std::cerr << "[ SharedCache ] Error: Not a valid Cache Type" << std::endl;
      exit(1);
    }
  }

  debug = false;
  is_default = true; // indication for default setup
  if (XML->sys.Embedded) {
    interface_ip.wt = Global_30;
    interface_ip.wire_is_mat_type = 0;
    interface_ip.wire_os_mat_type = 1;
  } else {
    interface_ip.wt = Global;
    interface_ip.wire_is_mat_type = 2;
    interface_ip.wire_os_mat_type = 2;
  }

  // All lower level cache are physically indexed and tagged.
  size = cachep.capacity;
  line = cachep.blockW;
  assoc = cachep.assoc;
  banks = cachep.nbanks;

  if ((cachep.dir_ty == ST && cacheL == L1Directory) ||
      (cachep.dir_ty == ST && cacheL == L2Directory)) {
    assoc = 0;
    tag = XML->sys.physical_address_width + EXTRA_TAG_BITS;
    interface_ip.num_search_ports = 1;
  } else {
    idx = debug ? 9 : int(ceil(log2(size / line / assoc)));
    tag = debug ? 51
                : XML->sys.physical_address_width - idx -
                      int(ceil(log2(line))) + EXTRA_TAG_BITS;
    interface_ip.num_search_ports = 0;
    if (cachep.dir_ty == SBT) {
      dir_overhead =
          ceil(XML->sys.number_of_cores / 8.0) * 8 / (cachep.blockW * 8);
      line = cachep.blockW * (1 + dir_overhead);
      size = cachep.capacity * (1 + dir_overhead);
    }
  }
  //  if (XML->sys.first_level_dir==2)
  //	  tag += int(XML->sys.domain_size + 5);
  interface_ip.specific_tag = 1;
  interface_ip.tag_w = tag;
  interface_ip.cache_sz = (int)size;
  interface_ip.line_sz = (int)line;
  interface_ip.assoc = (int)assoc;
  interface_ip.nbanks = (int)banks;
  interface_ip.out_w = interface_ip.line_sz * 8 / 2;
  interface_ip.access_mode = 1;
  interface_ip.throughput = cachep.throughput;
  interface_ip.latency = cachep.latency;
  interface_ip.is_cache = true;
  interface_ip.pure_ram = false;
  interface_ip.pure_cam = false;
  interface_ip.obj_func_dyn_energy = 0;
  interface_ip.obj_func_dyn_power = 0;
  interface_ip.obj_func_leak_power = 0;
  interface_ip.obj_func_cycle_t = 1;
  interface_ip.num_rw_ports = 1; // lower level cache usually has one port.
  interface_ip.num_rd_ports = 0;
  interface_ip.num_wr_ports = 0;
  interface_ip.num_se_rd_ports = 0;
  //  interface_ip.force_cache_config  =true;
  //  interface_ip.ndwl = 4;
  //  interface_ip.ndbl = 8;
  //  interface_ip.nspd = 1;
  //  interface_ip.ndcm =1 ;
  //  interface_ip.ndsam1 =1;
  //  interface_ip.ndsam2 =1;
  unicache.caches.set_params(
      &interface_ip, cachep.name + "cache", device_t, true, core_t);

  if (!((cachep.dir_ty == ST && cacheL == L1Directory) ||
        (cachep.dir_ty == ST && cacheL == L2Directory))) {
    tag = XML->sys.physical_address_width + EXTRA_TAG_BITS;
    data = (XML->sys.physical_address_width) + int(ceil(log2(size / line))) +
           unicache.caches.l_ip.line_sz;
    interface_ip.specific_tag = 1;
    interface_ip.tag_w = tag;
    interface_ip.line_sz =
        int(ceil(data / 8.0)); // int(ceil(pow(2.0,ceil(log2(data)))/8.0));
    interface_ip.cache_sz = cachep.missb_size * interface_ip.line_sz;
    interface_ip.assoc = 0;
    interface_ip.is_cache = true;
    interface_ip.pure_ram = false;
    interface_ip.pure_cam = false;
    interface_ip.nbanks = 1;
    interface_ip.out_w = interface_ip.line_sz * 8 / 2;
    interface_ip.access_mode = 0;
    interface_ip.throughput = cachep.throughput; // means cycle time
    interface_ip.latency = cachep.latency;       // means access time
    interface_ip.obj_func_dyn_energy = 0;
    interface_ip.obj_func_dyn_power = 0;
    interface_ip.obj_func_leak_power = 0;
    interface_ip.obj_func_cycle_t = 1;
    interface_ip.num_rw_ports = 1;
    interface_ip.num_rd_ports = 0;
    interface_ip.num_wr_ports = 0;
    interface_ip.num_se_rd_ports = 0;
    interface_ip.num_search_ports = 1;
    unicache.missb.set_params(
        &interface_ip, cachep.name + "MissB", device_t, true, core_t);

    // fill buffer
    tag = XML->sys.physical_address_width + EXTRA_TAG_BITS;
    data = unicache.caches.l_ip.line_sz;
    interface_ip.specific_tag = 1;
    interface_ip.tag_w = tag;
    interface_ip.line_sz = data; // int(pow(2.0,ceil(log2(data))));
    interface_ip.cache_sz = data * cachep.fu_size;
    interface_ip.assoc = 0;
    interface_ip.nbanks = 1;
    interface_ip.out_w = interface_ip.line_sz * 8 / 2;
    interface_ip.access_mode = 0;
    interface_ip.throughput = cachep.throughput;
    interface_ip.latency = cachep.latency;
    interface_ip.obj_func_dyn_energy = 0;
    interface_ip.obj_func_dyn_power = 0;
    interface_ip.obj_func_leak_power = 0;
    interface_ip.obj_func_cycle_t = 1;
    interface_ip.num_rw_ports = 1;
    interface_ip.num_rd_ports = 0;
    interface_ip.num_wr_ports = 0;
    interface_ip.num_se_rd_ports = 0;
    unicache.ifb.set_params(
        &interface_ip, cachep.name + "FillB", device_t, true, core_t);

    // prefetch buffer
    tag = XML->sys.physical_address_width +
          EXTRA_TAG_BITS; // check with previous entries to decide wthether to
                          // merge.
    data = unicache.caches.l_ip
               .line_sz; // separate queue to prevent from cache polution.
    interface_ip.specific_tag = 1;
    interface_ip.tag_w = tag;
    interface_ip.line_sz = data; // int(pow(2.0,ceil(log2(data))));
    interface_ip.cache_sz = cachep.prefetchb_size * interface_ip.line_sz;
    interface_ip.assoc = 0;
    interface_ip.nbanks = 1;
    interface_ip.out_w = interface_ip.line_sz * 8 / 2;
    interface_ip.access_mode = 0;
    interface_ip.throughput = cachep.throughput;
    interface_ip.latency = cachep.latency;
    interface_ip.obj_func_dyn_energy = 0;
    interface_ip.obj_func_dyn_power = 0;
    interface_ip.obj_func_leak_power = 0;
    interface_ip.obj_func_cycle_t = 1;
    interface_ip.num_rw_ports = 1;
    interface_ip.num_rd_ports = 0;
    interface_ip.num_wr_ports = 0;
    interface_ip.num_se_rd_ports = 0;
    unicache.prefetchb.set_params(
        &interface_ip, cachep.name + "PrefetchB", device_t, true, core_t);

    // WBB
    tag = XML->sys.physical_address_width + EXTRA_TAG_BITS;
    data = unicache.caches.l_ip.line_sz;
    interface_ip.specific_tag = 1;
    interface_ip.tag_w = tag;
    interface_ip.line_sz = data;
    interface_ip.cache_sz = cachep.wbb_size * interface_ip.line_sz;
    interface_ip.assoc = 0;
    interface_ip.nbanks = 1;
    interface_ip.out_w = interface_ip.line_sz * 8 / 2;
    interface_ip.access_mode = 0;
    interface_ip.throughput = cachep.throughput;
    interface_ip.latency = cachep.latency;
    interface_ip.obj_func_dyn_energy = 0;
    interface_ip.obj_func_dyn_power = 0;
    interface_ip.obj_func_leak_power = 0;
    interface_ip.obj_func_cycle_t = 1;
    interface_ip.num_rw_ports = 1;
    interface_ip.num_rd_ports = 0;
    interface_ip.num_wr_ports = 0;
    interface_ip.num_se_rd_ports = 0;
    unicache.wbb.set_params(
        &interface_ip, cachep.name + "WBB", device_t, true, core_t);
  }
  init_params = true;
}

void SharedCache::set_stats(const ParseXML *XML) {
  this->XML = XML;
  init_stats = true;
}

void SharedCache::computeArea() {
  if (!init_params) {
    std::cerr << "[ SharedCache ] Error: must set params before calling "
                 "computeArea()\n";
    exit(1);
  }

  unicache.caches.computeArea();
  unicache.area.set_area(unicache.area.get_area() +
                         unicache.caches.local_result.area);
  area.set_area(area.get_area() + unicache.caches.local_result.area);
  interface_ip.force_cache_config = false;

  if (unicache.caches.local_result.tag_array2 != nullptr) {
    unicache.caches.local_result.ta2_power =
        unicache.caches.local_result.tag_array2->power;
  }
  if (unicache.caches.local_result.data_array2 != nullptr) {
    unicache.caches.local_result.da2_power =
        unicache.caches.local_result.data_array2->power;
  }

  if (!((cachep.dir_ty == ST && cacheL == L1Directory) ||
        (cachep.dir_ty == ST && cacheL == L2Directory))) {
    unicache.missb.computeArea();
    unicache.area.set_area(unicache.area.get_area() +
                           unicache.missb.local_result.area);
    area.set_area(area.get_area() + unicache.missb.local_result.area);
    if (unicache.missb.local_result.tag_array2 != nullptr) {
      unicache.missb.local_result.ta2_power =
          unicache.missb.local_result.tag_array2->power;
    }
    if (unicache.missb.local_result.data_array2 != nullptr) {
      unicache.missb.local_result.da2_power =
          unicache.missb.local_result.data_array2->power;
    }

    // Fill Buffer:
    unicache.ifb.computeArea();
    unicache.area.set_area(unicache.area.get_area() +
                           unicache.ifb.local_result.area);
    area.set_area(area.get_area() + unicache.ifb.local_result.area);
    if (unicache.ifb.local_result.tag_array2 != nullptr) {
      unicache.ifb.local_result.ta2_power =
          unicache.ifb.local_result.tag_array2->power;
    }
    if (unicache.ifb.local_result.data_array2 != nullptr) {
      unicache.ifb.local_result.da2_power =
          unicache.ifb.local_result.data_array2->power;
    }

    // Prefetch Buffer:
    unicache.prefetchb.computeArea();
    unicache.area.set_area(unicache.area.get_area() +
                           unicache.prefetchb.local_result.area);
    area.set_area(area.get_area() + unicache.prefetchb.local_result.area);
    if (unicache.prefetchb.local_result.tag_array2 != nullptr) {
      unicache.prefetchb.local_result.ta2_power =
          unicache.prefetchb.local_result.tag_array2->power;
    }
    if (unicache.prefetchb.local_result.data_array2 != nullptr) {
      unicache.prefetchb.local_result.da2_power =
          unicache.prefetchb.local_result.data_array2->power;
    }

    // WBB:
    unicache.wbb.computeArea();
    unicache.area.set_area(unicache.area.get_area() +
                           unicache.wbb.local_result.area);
    area.set_area(area.get_area() + unicache.wbb.local_result.area);
    if (unicache.wbb.local_result.tag_array2 != nullptr) {
      unicache.wbb.local_result.ta2_power =
          unicache.wbb.local_result.tag_array2->power;
    }
    if (unicache.wbb.local_result.data_array2 != nullptr) {
      unicache.wbb.local_result.da2_power =
          unicache.wbb.local_result.data_array2->power;
    }
  }
  set_area = true;
}

void SharedCache::computeStaticPower(bool is_tdp) {
  if (!init_params) {
    std::cerr << "[ SharedCache ] Error: must set params before calling "
                 "computeStaticPower()\n";
    exit(1);
  }
  if (!set_area) {
    std::cerr << "[ SharedCache ] Error: must ComputeArea before calling "
                 "computeStaticPower()\n";
    exit(1);
  }
  if (is_tdp) {
    power.reset();
    rt_power.reset();
  }
  double homenode_data_access = (cachep.dir_ty == SBT) ? 0.9 : 1.0;
  if (is_tdp) {
    if (!((cachep.dir_ty == ST && cacheL == L1Directory) ||
          (cachep.dir_ty == ST && cacheL == L2Directory))) {
      // init stats for Peak
      unicache.caches.stats_t.readAc.access =
          .67 * unicache.caches.l_ip.num_rw_ports * cachep.duty_cycle *
          homenode_data_access;
      // std::cout << unicache.caches.stats_t.readAc.access << "\n";
      unicache.caches.stats_t.readAc.miss = 0;
      // std::cout << unicache.caches.stats_t.readAc.miss << "\n";
      unicache.caches.stats_t.readAc.hit =
          unicache.caches.stats_t.readAc.access -
          unicache.caches.stats_t.readAc.miss;
      // std::cout << unicache.caches.stats_t.readAc.hit << "\n";
      unicache.caches.stats_t.writeAc.access =
          .33 * unicache.caches.l_ip.num_rw_ports * cachep.duty_cycle *
          homenode_data_access;
      // std::cout << unicache.caches.stats_t.writeAc.access << "\n";
      unicache.caches.stats_t.writeAc.miss = 0;
      // std::cout << unicache.caches.stats_t.writeAc.miss << "\n";
      unicache.caches.stats_t.writeAc.hit =
          unicache.caches.stats_t.writeAc.access -
          unicache.caches.stats_t.writeAc.miss;
      // std::cout << unicache.caches.stats_t.writeAc.hit << "\n";
      unicache.caches.tdp_stats = unicache.caches.stats_t;

      if (cachep.dir_ty == SBT) {
        homenode_stats_t.readAc.access =
            .67 * unicache.caches.l_ip.num_rw_ports * cachep.dir_duty_cycle *
            (1 - homenode_data_access);
        // std::cout << homenode_stats_t.readAc.access << "\n";
        homenode_stats_t.readAc.miss = 0;
        // std::cout << homenode_stats_t.readAc.miss << "\n";
        homenode_stats_t.readAc.hit =
            homenode_stats_t.readAc.access - homenode_stats_t.readAc.miss;
        // std::cout << homenode_stats_t.readAc.hit << "\n";
        homenode_stats_t.writeAc.access =
            .67 * unicache.caches.l_ip.num_rw_ports * cachep.dir_duty_cycle *
            (1 - homenode_data_access);
        // std::cout << homenode_stats_t.writeAc.access << "\n";
        homenode_stats_t.writeAc.miss = 0;
        // std::cout << homenode_stats_t.writeAc.miss << "\n";
        homenode_stats_t.writeAc.hit =
            homenode_stats_t.writeAc.access - homenode_stats_t.writeAc.miss;
        // std::cout << homenode_stats_t.writeAc.hit << "\n";
        homenode_tdp_stats = homenode_stats_t;
      }

      unicache.missb.stats_t.readAc.access =
          unicache.missb.l_ip.num_search_ports * cachep.duty_cycle;
      // std::cout << "unicache.missb.stats_t.readAc.access";
      // std::cout << unicache.missb.stats_t.readAc.access << "\n";
      unicache.missb.stats_t.writeAc.access =
          unicache.missb.l_ip.num_search_ports * cachep.duty_cycle;
      // std::cout << "unicache.missb.stats_t.writeAc.access";
      // std::cout << unicache.missb.stats_t.writeAc.access << "\n";
      unicache.missb.tdp_stats = unicache.missb.stats_t;

      unicache.ifb.stats_t.readAc.access =
          unicache.ifb.l_ip.num_search_ports * cachep.duty_cycle;
      // std::cout << "unicache.ifb.stats_t.readAc.access";
      // std::cout << unicache.ifb.stats_t.readAc.access << "\n";
      unicache.ifb.stats_t.writeAc.access =
          unicache.ifb.l_ip.num_search_ports * cachep.duty_cycle;
      // std::cout << "unicache.ifb.stats_t.writeAc.access";
      // std::cout << unicache.ifb.stats_t.writeAc.access << "\n";
      unicache.ifb.tdp_stats = unicache.ifb.stats_t;

      unicache.prefetchb.stats_t.readAc.access =
          unicache.prefetchb.l_ip.num_search_ports * cachep.duty_cycle;
      // std::cout << "unicache.prefetchb.stats_t.readAc.access";
      // std::cout << unicache.prefetchb.stats_t.readAc.access << "\n";
      unicache.prefetchb.stats_t.writeAc.access =
          unicache.ifb.l_ip.num_search_ports * cachep.duty_cycle;
      // std::cout << "unicache.prefetchb.stats_t.writeAc.access";
      // std::cout << unicache.prefetchb.stats_t.writeAc.access << "\n";
      unicache.prefetchb.tdp_stats = unicache.prefetchb.stats_t;

      unicache.wbb.stats_t.readAc.access =
          unicache.wbb.l_ip.num_search_ports * cachep.duty_cycle;
      // std::cout << "unicache.wbb.stats_t.readAc.access";
      // std::cout << unicache.wbb.stats_t.readAc.access << "\n";
      unicache.wbb.stats_t.writeAc.access =
          unicache.wbb.l_ip.num_search_ports * cachep.duty_cycle;
      // std::cout << "unicache.wbb.stats_t.writeAc.access";
      // std::cout << unicache.wbb.stats_t.writeAc.access << "\n";
      unicache.wbb.tdp_stats = unicache.wbb.stats_t;
    } else {
      unicache.caches.stats_t.readAc.access =
          unicache.caches.l_ip.num_search_ports * cachep.duty_cycle;
      // std::cout << unicache.caches.stats_t.readAc.access << "\n";
      unicache.caches.stats_t.readAc.miss = 0;
      // std::cout << unicache.caches.stats_t.readAc.miss << "\n";
      unicache.caches.stats_t.readAc.hit =
          unicache.caches.stats_t.readAc.access -
          unicache.caches.stats_t.readAc.miss;
      // std::cout << unicache.caches.stats_t.readAc.hit << "\n";
      unicache.caches.stats_t.writeAc.access = 0;
      // std::cout << unicache.caches.stats_t.writeAc.access << "\n";
      unicache.caches.stats_t.writeAc.miss = 0;
      // std::cout << unicache.caches.stats_t.writeAc.miss << "\n";
      unicache.caches.stats_t.writeAc.hit =
          unicache.caches.stats_t.writeAc.access -
          unicache.caches.stats_t.writeAc.miss;
      // std::cout << unicache.caches.stats_t.writeAc.hit << "\n";
      unicache.caches.tdp_stats = unicache.caches.stats_t;
      // std::cout << unicache.caches.stats_t.writeAc.hit << "\n";
    }

  } else {
    // init stats for runtime power (RTP)
    if (cacheL == L2) {
      unicache.caches.stats_t.readAc.access =
          XML->sys.L2[ithCache].read_accesses;
      unicache.caches.stats_t.readAc.miss = XML->sys.L2[ithCache].read_misses;
      unicache.caches.stats_t.readAc.hit =
          unicache.caches.stats_t.readAc.access -
          unicache.caches.stats_t.readAc.miss;
      unicache.caches.stats_t.writeAc.access =
          XML->sys.L2[ithCache].write_accesses;
      unicache.caches.stats_t.writeAc.miss = XML->sys.L2[ithCache].write_misses;
      unicache.caches.stats_t.writeAc.hit =
          unicache.caches.stats_t.writeAc.access -
          unicache.caches.stats_t.writeAc.miss;
      unicache.caches.rtp_stats = unicache.caches.stats_t;

      if (cachep.dir_ty == SBT) {
        homenode_rtp_stats.readAc.access =
            XML->sys.L2[ithCache].homenode_read_accesses;
        homenode_rtp_stats.readAc.miss =
            XML->sys.L2[ithCache].homenode_read_misses;
        homenode_rtp_stats.readAc.hit =
            homenode_rtp_stats.readAc.access - homenode_rtp_stats.readAc.miss;
        homenode_rtp_stats.writeAc.access =
            XML->sys.L2[ithCache].homenode_write_accesses;
        homenode_rtp_stats.writeAc.miss =
            XML->sys.L2[ithCache].homenode_write_misses;
        homenode_rtp_stats.writeAc.hit =
            homenode_rtp_stats.writeAc.access - homenode_rtp_stats.writeAc.miss;
      }
    } else if (cacheL == L3) {
      unicache.caches.stats_t.readAc.access =
          XML->sys.L3[ithCache].read_accesses;
      unicache.caches.stats_t.readAc.miss = XML->sys.L3[ithCache].read_misses;
      unicache.caches.stats_t.readAc.hit =
          unicache.caches.stats_t.readAc.access -
          unicache.caches.stats_t.readAc.miss;
      unicache.caches.stats_t.writeAc.access =
          XML->sys.L3[ithCache].write_accesses;
      unicache.caches.stats_t.writeAc.miss = XML->sys.L3[ithCache].write_misses;
      unicache.caches.stats_t.writeAc.hit =
          unicache.caches.stats_t.writeAc.access -
          unicache.caches.stats_t.writeAc.miss;
      unicache.caches.rtp_stats = unicache.caches.stats_t;

      if (cachep.dir_ty == SBT) {
        homenode_rtp_stats.readAc.access =
            XML->sys.L3[ithCache].homenode_read_accesses;
        homenode_rtp_stats.readAc.miss =
            XML->sys.L3[ithCache].homenode_read_misses;
        homenode_rtp_stats.readAc.hit =
            homenode_rtp_stats.readAc.access - homenode_rtp_stats.readAc.miss;
        homenode_rtp_stats.writeAc.access =
            XML->sys.L3[ithCache].homenode_write_accesses;
        homenode_rtp_stats.writeAc.miss =
            XML->sys.L3[ithCache].homenode_write_misses;
        homenode_rtp_stats.writeAc.hit =
            homenode_rtp_stats.writeAc.access - homenode_rtp_stats.writeAc.miss;
      }
    } else if (cacheL == L1Directory) {
      unicache.caches.stats_t.readAc.access =
          XML->sys.L1Directory[ithCache].read_accesses;
      unicache.caches.stats_t.readAc.miss =
          XML->sys.L1Directory[ithCache].read_misses;
      unicache.caches.stats_t.readAc.hit =
          unicache.caches.stats_t.readAc.access -
          unicache.caches.stats_t.readAc.miss;
      unicache.caches.stats_t.writeAc.access =
          XML->sys.L1Directory[ithCache].write_accesses;
      unicache.caches.stats_t.writeAc.miss =
          XML->sys.L1Directory[ithCache].write_misses;
      unicache.caches.stats_t.writeAc.hit =
          unicache.caches.stats_t.writeAc.access -
          unicache.caches.stats_t.writeAc.miss;
      unicache.caches.rtp_stats = unicache.caches.stats_t;
    } else if (cacheL == L2Directory) {
      unicache.caches.stats_t.readAc.access =
          XML->sys.L2Directory[ithCache].read_accesses;
      unicache.caches.stats_t.readAc.miss =
          XML->sys.L2Directory[ithCache].read_misses;
      unicache.caches.stats_t.readAc.hit =
          unicache.caches.stats_t.readAc.access -
          unicache.caches.stats_t.readAc.miss;
      unicache.caches.stats_t.writeAc.access =
          XML->sys.L2Directory[ithCache].write_accesses;
      unicache.caches.stats_t.writeAc.miss =
          XML->sys.L2Directory[ithCache].write_misses;
      unicache.caches.stats_t.writeAc.hit =
          unicache.caches.stats_t.writeAc.access -
          unicache.caches.stats_t.writeAc.miss;
      unicache.caches.rtp_stats = unicache.caches.stats_t;
    }
    if (!((cachep.dir_ty == ST && cacheL == L1Directory) ||
          (cachep.dir_ty == ST &&
           cacheL ==
               L2Directory))) { // Assuming write back and write-allocate cache

      unicache.missb.stats_t.readAc.access =
          unicache.caches.stats_t.writeAc.miss;
      unicache.missb.stats_t.writeAc.access =
          unicache.caches.stats_t.writeAc.miss;
      unicache.missb.rtp_stats = unicache.missb.stats_t;

      unicache.ifb.stats_t.readAc.access = unicache.caches.stats_t.writeAc.miss;
      unicache.ifb.stats_t.writeAc.access =
          unicache.caches.stats_t.writeAc.miss;
      unicache.ifb.rtp_stats = unicache.ifb.stats_t;

      unicache.prefetchb.stats_t.readAc.access =
          unicache.caches.stats_t.writeAc.miss;
      unicache.prefetchb.stats_t.writeAc.access =
          unicache.caches.stats_t.writeAc.miss;
      unicache.prefetchb.rtp_stats = unicache.prefetchb.stats_t;

      unicache.wbb.stats_t.readAc.access = unicache.caches.stats_t.writeAc.miss;
      unicache.wbb.stats_t.writeAc.access =
          unicache.caches.stats_t.writeAc.miss;
      if (cachep.dir_ty == SBT) {
        unicache.missb.stats_t.readAc.access += homenode_rtp_stats.writeAc.miss;
        unicache.missb.stats_t.writeAc.access +=
            homenode_rtp_stats.writeAc.miss;
        unicache.missb.rtp_stats = unicache.missb.stats_t;

        unicache.missb.stats_t.readAc.access += homenode_rtp_stats.writeAc.miss;
        unicache.missb.stats_t.writeAc.access +=
            homenode_rtp_stats.writeAc.miss;
        unicache.missb.rtp_stats = unicache.missb.stats_t;

        unicache.ifb.stats_t.readAc.access += homenode_rtp_stats.writeAc.miss;
        unicache.ifb.stats_t.writeAc.access += homenode_rtp_stats.writeAc.miss;
        unicache.ifb.rtp_stats = unicache.ifb.stats_t;

        unicache.prefetchb.stats_t.readAc.access +=
            homenode_rtp_stats.writeAc.miss;
        unicache.prefetchb.stats_t.writeAc.access +=
            homenode_rtp_stats.writeAc.miss;
        unicache.prefetchb.rtp_stats = unicache.prefetchb.stats_t;

        unicache.wbb.stats_t.readAc.access += homenode_rtp_stats.writeAc.miss;
        unicache.wbb.stats_t.writeAc.access += homenode_rtp_stats.writeAc.miss;
      }
      unicache.wbb.rtp_stats = unicache.wbb.stats_t;
    }
  }

  unicache.power_t.reset();
  if (!((cachep.dir_ty == ST && cacheL == L1Directory) ||
        (cachep.dir_ty == ST && cacheL == L2Directory))) {
    unicache.power_t.readOp.dynamic +=
        (unicache.caches.stats_t.readAc.hit *
             unicache.caches.local_result.power.readOp.dynamic +
         unicache.caches.stats_t.readAc.miss *
             unicache.caches.local_result.ta2_power.readOp.dynamic +
         unicache.caches.stats_t.writeAc.miss *
             unicache.caches.local_result.ta2_power.writeOp.dynamic +
         unicache.caches.stats_t.writeAc.access *
             unicache.caches.local_result.power.writeOp
                 .dynamic); // write miss will also generate a write later
    // std::cout << "unicache.caches.local_result.power.readOp.dynamic ";
    // std::cout << unicache.caches.local_result.power.readOp.dynamic << "\n";
    // std::cout << "unicache.caches.local_result.ta2_power.readOp.dynamic ";
    // std::cout << unicache.caches.local_result.ta2_power.readOp.dynamic <<
    // "\n"; std::cout <<
    // "unicache.caches.local_result.ta2_power.writeOp.dynamic
    // "; std::cout << unicache.caches.local_result.ta2_power.writeOp.dynamic <<
    // "\n"; std::cout << "unicache.caches.local_result.power.writeOp.dynamic ";
    // std::cout << unicache.caches.local_result.power.writeOp.dynamic << "\n";
    // std::cout << "unicache.power_t.readOp.dynamic ";
    // std::cout << unicache.power_t.readOp.dynamic << "\n";

    if (cachep.dir_ty == SBT) {
      unicache.power_t.readOp.dynamic +=
          homenode_stats_t.readAc.hit *
              (unicache.caches.local_result.da2_power.readOp.dynamic *
                   dir_overhead +
               unicache.caches.local_result.ta2_power.readOp.dynamic) +
          homenode_stats_t.readAc.miss *
              unicache.caches.local_result.ta2_power.readOp.dynamic +
          homenode_stats_t.writeAc.miss *
              unicache.caches.local_result.ta2_power.readOp.dynamic +
          homenode_stats_t.writeAc.hit *
              (unicache.caches.local_result.da2_power.writeOp.dynamic *
                   dir_overhead +
               unicache.caches.local_result.ta2_power.readOp.dynamic +
               homenode_stats_t.writeAc.miss *
                   unicache.caches.local_result.power.writeOp
                       .dynamic); // write miss on dynamic home node will
                                  // generate a replacement write on whole cache
                                  // block
      // std::cout << "unicache.power_t.readOp.dynamic ";
      // std::cout << unicache.power_t.readOp.dynamic << "\n";
    }

    unicache.power_t.readOp.dynamic +=
        unicache.missb.stats_t.readAc.access *
            unicache.missb.local_result.power.searchOp.dynamic +
        unicache.missb.stats_t.writeAc.access *
            unicache.missb.local_result.power.writeOp
                .dynamic; // each access to missb involves a CAM and a write
    // std::cout << "unicache.power_t.readOp.dynamic ";
    // std::cout << unicache.power_t.readOp.dynamic << "\n";
    unicache.power_t.readOp.dynamic +=
        unicache.ifb.stats_t.readAc.access *
            unicache.ifb.local_result.power.searchOp.dynamic +
        unicache.ifb.stats_t.writeAc.access *
            unicache.ifb.local_result.power.writeOp.dynamic;
    // std::cout << "unicache.power_t.readOp.dynamic ";
    // std::cout << unicache.power_t.readOp.dynamic << "\n";
    unicache.power_t.readOp.dynamic +=
        unicache.prefetchb.stats_t.readAc.access *
            unicache.prefetchb.local_result.power.searchOp.dynamic +
        unicache.prefetchb.stats_t.writeAc.access *
            unicache.prefetchb.local_result.power.writeOp.dynamic;
    // std::cout << "unicache.power_t.readOp.dynamic ";
    // std::cout << unicache.power_t.readOp.dynamic << "\n";
    unicache.power_t.readOp.dynamic +=
        unicache.wbb.stats_t.readAc.access *
            unicache.wbb.local_result.power.searchOp.dynamic +
        unicache.wbb.stats_t.writeAc.access *
            unicache.wbb.local_result.power.writeOp.dynamic;
    // std::cout << "unicache.power_t.readOp.dynamic ";
    // std::cout << unicache.power_t.readOp.dynamic << "\n";
  } else {
    unicache.power_t.readOp.dynamic +=
        (unicache.caches.stats_t.readAc.access *
             unicache.caches.local_result.power.searchOp.dynamic +
         unicache.caches.stats_t.writeAc.access *
             unicache.caches.local_result.power.writeOp.dynamic);
    // std::cout << "unicache.power_t.readOp.dynamic ";
    // std::cout << unicache.power_t.readOp.dynamic << "\n";
  }

  if (is_tdp) {
    unicache.power =
        unicache.power_t + (unicache.caches.local_result.power) * pppm_lkg;
    if (!((cachep.dir_ty == ST && cacheL == L1Directory) ||
          (cachep.dir_ty == ST && cacheL == L2Directory))) {
      unicache.power = unicache.power + (unicache.missb.local_result.power +
                                         unicache.ifb.local_result.power +
                                         unicache.prefetchb.local_result.power +
                                         unicache.wbb.local_result.power) *
                                            pppm_lkg;
    }
    power = power + unicache.power;
    //		cout<<"unicache.caches.local_result.power.readOp.dynamic"<<unicache.caches.local_result.power.readOp.dynamic<<endl;
    //		cout<<"unicache.caches.local_result.power.writeOp.dynamic"<<unicache.caches.local_result.power.writeOp.dynamic<<endl;
  } else {
    unicache.rt_power =
        unicache.power_t + (unicache.caches.local_result.power) * pppm_lkg;
    if (!((cachep.dir_ty == ST && cacheL == L1Directory) ||
          (cachep.dir_ty == ST && cacheL == L2Directory))) {
      unicache.rt_power =
          unicache.rt_power +
          (unicache.missb.local_result.power + unicache.ifb.local_result.power +
           unicache.prefetchb.local_result.power +
           unicache.wbb.local_result.power) *
              pppm_lkg;
    }
    rt_power = rt_power + unicache.rt_power;
  }
}

void SharedCache::display(uint32_t indent, bool is_tdp) {
  string indent_str(indent, ' ');
  string indent_str_next(indent + 2, ' ');
  bool long_channel = XML->sys.longer_channel_device;
  bool power_gating = XML->sys.power_gating;

  if (is_tdp) {
    cout << (XML->sys.Private_L2 ? indent_str : "") << cachep.name << endl;
    cout << indent_str << "Area = " << area.get_area() * 1e-6 << " mm^2"
         << endl;
    cout << indent_str
         << "Peak Dynamic = " << power.readOp.dynamic * cachep.clockRate << " W"
         << endl;
    cout << indent_str << "Subthreshold Leakage = "
         << (long_channel ? power.readOp.longer_channel_leakage
                          : power.readOp.leakage)
         << " W" << endl;
    if (power_gating)
      cout << indent_str << "Subthreshold Leakage with power gating = "
           << (long_channel ? power.readOp.power_gated_with_long_channel_leakage
                            : power.readOp.power_gated_leakage)
           << " W" << endl;
    cout << indent_str << "Gate Leakage = " << power.readOp.gate_leakage << " W"
         << endl;
    cout << indent_str << "Runtime Dynamic = "
         << rt_power.readOp.dynamic / cachep.executionTime << " W" << endl;
    cout << endl;
  } else {
  }
}

// void SharedCache::computeMaxPower()
//{
//  //Compute maximum power and runtime power.
//  //When computing runtime power, McPAT gets or reasons out the statistics
//  based on XML input. maxPower		= 0.0;
//  //llCache,itlb
//  llCache.maxPower   = 0.0;
//  llCache.maxPower	+=
//  (llCache.caches.l_ip.num_rw_ports*(0.67*llCache.caches.local_result.power.readOp.dynamic+0.33*llCache.caches.local_result.power.writeOp.dynamic)
//                        +llCache.caches.l_ip.num_rd_ports*llCache.caches.local_result.power.readOp.dynamic+llCache.caches.l_ip.num_wr_ports*llCache.caches.local_result.power.writeOp.dynamic
//                        +llCache.caches.l_ip.num_se_rd_ports*llCache.caches.local_result.power.readOp.dynamic)*clockRate;
//  ///cout<<"llCache.maxPower=" <<llCache.maxPower<<endl;
//
//  llCache.maxPower	+=
//  llCache.missb.l_ip.num_search_ports*llCache.missb.local_result.power.searchOp.dynamic*clockRate;
//  ///cout<<"llCache.maxPower=" <<llCache.maxPower<<endl;
//
//  llCache.maxPower	+=
//  llCache.ifb.l_ip.num_search_ports*llCache.ifb.local_result.power.searchOp.dynamic*clockRate;
//  ///cout<<"llCache.maxPower=" <<llCache.maxPower<<endl;
//
//  llCache.maxPower	+=
//  llCache.prefetchb.l_ip.num_search_ports*llCache.prefetchb.local_result.power.searchOp.dynamic*clockRate;
//  ///cout<<"llCache.maxPower=" <<llCache.maxPower<<endl;
//
//  llCache.maxPower	+=
//  llCache.wbb.l_ip.num_search_ports*llCache.wbb.local_result.power.searchOp.dynamic*clockRate;
//  //llCache.maxPower *=  scktRatio; //TODO: this calculation should be
//  self-contained
//  ///cout<<"llCache.maxPower=" <<llCache.maxPower<<endl;
//
////  directory_power =
///(directory.caches.l_ip.num_rw_ports*(0.67*directory.caches.local_result.power.readOp.dynamic+0.33*directory.caches.local_result.power.writeOp.dynamic)
////
///+directory.caches.l_ip.num_rd_ports*directory.caches.local_result.power.readOp.dynamic+directory.caches.l_ip.num_wr_ports*directory.caches.local_result.power.writeOp.dynamic
////
///+directory.caches.l_ip.num_se_rd_ports*directory.caches.local_result.power.readOp.dynamic)*clockRate;
//
//  L2Tot.power.readOp.dynamic = llCache.maxPower;
//  L2Tot.power.readOp.leakage =
//  llCache.caches.local_result.power.readOp.leakage +
//                               llCache.missb.local_result.power.readOp.leakage
//                               + llCache.ifb.local_result.power.readOp.leakage
//                               +
//                               llCache.prefetchb.local_result.power.readOp.leakage
//                               +
//                               llCache.wbb.local_result.power.readOp.leakage;
//
//  L2Tot.area.set_area(llCache.area*1.1*1e-6);//placement and routing overhead
//
//  if (XML->sys.number_of_dir_levels==1)
//  {
//	  if (XML->sys.first_level_dir==0)
//	  {
//		  directory.maxPower   = 0.0;
//		  directory.maxPower	+=
//(directory.caches.l_ip.num_rw_ports*(0.67*directory.caches.local_result.power.readOp.dynamic+0.33*directory.caches.local_result.power.writeOp.dynamic)
//		                        +directory.caches.l_ip.num_rd_ports*directory.caches.local_result.power.readOp.dynamic+directory.caches.l_ip.num_wr_ports*directory.caches.local_result.power.writeOp.dynamic
//		                        +directory.caches.l_ip.num_se_rd_ports*directory.caches.local_result.power.readOp.dynamic)*clockRate;
//		  ///cout<<"directory.maxPower=" <<directory.maxPower<<endl;
//
//		  directory.maxPower	+=
// directory.missb.l_ip.num_search_ports*directory.missb.local_result.power.searchOp.dynamic*clockRate;
//		  ///cout<<"directory.maxPower=" <<directory.maxPower<<endl;
//
//		  directory.maxPower	+=
// directory.ifb.l_ip.num_search_ports*directory.ifb.local_result.power.searchOp.dynamic*clockRate;
//		  ///cout<<"directory.maxPower=" <<directory.maxPower<<endl;
//
//		  directory.maxPower	+=
// directory.prefetchb.l_ip.num_search_ports*directory.prefetchb.local_result.power.searchOp.dynamic*clockRate;
//		  ///cout<<"directory.maxPower=" <<directory.maxPower<<endl;
//
//		  directory.maxPower	+=
// directory.wbb.l_ip.num_search_ports*directory.wbb.local_result.power.searchOp.dynamic*clockRate;
//
//		  cc.power.readOp.dynamic = directory.maxPower*scktRatio*8;//8
// is the memory controller counts 		  cc.power.readOp.leakage =
// directory.caches.local_result.power.readOp.leakage +
//                                     directory.missb.local_result.power.readOp.leakage
//                                     +
//                                     directory.ifb.local_result.power.readOp.leakage
//                                     +
//                                     directory.prefetchb.local_result.power.readOp.leakage
//                                     +
//                                     directory.wbb.local_result.power.readOp.leakage;
//
//		  cc.power.readOp.leakage *=8;
//
//		  cc.area.set_area(directory.area*8);
//		  cout<<"CC area="<<cc.area.get_area()*1e-6<<endl;
//		  cout<<"CC Power="<<cc.power.readOp.dynamic<<endl;
//		  ccTot.area.set_area(cc.area.get_area()*1e-6);
//		  ccTot.power = cc.power;
//		  cout<<"DC energy per access" <<
// cc.power.readOp.dynamic/clockRate/8;
//	  }
//	  else if (XML->sys.first_level_dir==1)
//	  {
//		  inv_dir.maxPower =
// inv_dir.caches.local_result.power.searchOp.dynamic*clockRate*XML->sys.domain_size;
//		  cc.power.readOp.dynamic  =
// inv_dir.maxPower*scktRatio*64/XML->sys.domain_size;
// cc.power.readOp.leakage  =
// inv_dir.caches.local_result.power.readOp.leakage*inv_dir.caches.l_ip.nbanks*64/XML->sys.domain_size;
//
//		  cc.area.set_area(inv_dir.area*64/XML->sys.domain_size);
//		  cout<<"CC area="<<cc.area.get_area()*1e-6<<endl;
//		  cout<<"CC Power="<<cc.power.readOp.dynamic<<endl;
//		  ccTot.area.set_area(cc.area.get_area()*1e-6);
//		  cout<<"DC energy per access" <<
// cc.power.readOp.dynamic/clockRate/8; 		  ccTot.power =
// cc.power;
//	  }
//  }
//
//  else if (XML->sys.number_of_dir_levels==2)
//  {
//
//	  		  directory.maxPower   = 0.0;
//	  		  directory.maxPower	+=
//(directory.caches.l_ip.num_rw_ports*(0.67*directory.caches.local_result.power.readOp.dynamic+0.33*directory.caches.local_result.power.writeOp.dynamic)
//	  		                        +directory.caches.l_ip.num_rd_ports*directory.caches.local_result.power.readOp.dynamic+directory.caches.l_ip.num_wr_ports*directory.caches.local_result.power.writeOp.dynamic
//	  		                        +directory.caches.l_ip.num_se_rd_ports*directory.caches.local_result.power.readOp.dynamic)*clockRate;
//	  		  ///cout<<"directory.maxPower="
//<<directory.maxPower<<endl;
//
//	  		  directory.maxPower	+=
// directory.missb.l_ip.num_search_ports*directory.missb.local_result.power.searchOp.dynamic*clockRate;
//	  		  ///cout<<"directory.maxPower="
//<<directory.maxPower<<endl;
//
//	  		  directory.maxPower	+=
// directory.ifb.l_ip.num_search_ports*directory.ifb.local_result.power.searchOp.dynamic*clockRate;
//	  		  ///cout<<"directory.maxPower="
//<<directory.maxPower<<endl;
//
//	  		  directory.maxPower	+=
// directory.prefetchb.l_ip.num_search_ports*directory.prefetchb.local_result.power.searchOp.dynamic*clockRate;
//	  		  ///cout<<"directory.maxPower="
//<<directory.maxPower<<endl;
//
//	  		  directory.maxPower	+=
// directory.wbb.l_ip.num_search_ports*directory.wbb.local_result.power.searchOp.dynamic*clockRate;
//
//	  		  cc.power.readOp.dynamic =
// directory.maxPower*scktRatio*8;//8 is the memory controller counts
//			  cc.power.readOp.leakage =
// directory.caches.local_result.power.readOp.leakage +
//	                                     directory.missb.local_result.power.readOp.leakage
//+ directory.ifb.local_result.power.readOp.leakage +
//	                                     directory.prefetchb.local_result.power.readOp.leakage
//+ directory.wbb.local_result.power.readOp.leakage;
// cc.power.readOp.leakage
//*=8; 	  		  cc.area.set_area(directory.area*8);
//
//	  		if (XML->sys.first_level_dir==0)
//	  		{
//	  		  directory1.maxPower   = 0.0;
//	  		  directory1.maxPower	+=
//(directory1.caches.l_ip.num_rw_ports*(0.67*directory1.caches.local_result.power.readOp.dynamic+0.33*directory1.caches.local_result.power.writeOp.dynamic)
//	  				  +directory1.caches.l_ip.num_rd_ports*directory1.caches.local_result.power.readOp.dynamic+directory1.caches.l_ip.num_wr_ports*directory1.caches.local_result.power.writeOp.dynamic
//	  				  +directory1.caches.l_ip.num_se_rd_ports*directory1.caches.local_result.power.readOp.dynamic)*clockRate;
//	  		  ///cout<<"directory1.maxPower="
//<<directory1.maxPower<<endl;
//
//	  		  directory1.maxPower	+=
// directory1.missb.l_ip.num_search_ports*directory1.missb.local_result.power.searchOp.dynamic*clockRate;
//	  		  ///cout<<"directory1.maxPower="
//<<directory1.maxPower<<endl;
//
//	  		  directory1.maxPower	+=
// directory1.ifb.l_ip.num_search_ports*directory1.ifb.local_result.power.searchOp.dynamic*clockRate;
//	  		  ///cout<<"directory1.maxPower="
//<<directory1.maxPower<<endl;
//
//	  		  directory1.maxPower	+=
// directory1.prefetchb.l_ip.num_search_ports*directory1.prefetchb.local_result.power.searchOp.dynamic*clockRate;
//	  		  ///cout<<"directory1.maxPower="
//<<directory1.maxPower<<endl;
//
//	  		  directory1.maxPower	+=
// directory1.wbb.l_ip.num_search_ports*directory1.wbb.local_result.power.searchOp.dynamic*clockRate;
//
//	  		  cc1.power.readOp.dynamic =
// directory1.maxPower*scktRatio*64/XML->sys.domain_size;
//			  cc1.power.readOp.leakage =
// directory1.caches.local_result.power.readOp.leakage +
//	                                     directory1.missb.local_result.power.readOp.leakage
//+ directory1.ifb.local_result.power.readOp.leakage +
//	                                     directory1.prefetchb.local_result.power.readOp.leakage
//+ directory1.wbb.local_result.power.readOp.leakage;
// cc1.power.readOp.leakage
//*= 64/XML->sys.domain_size;
//	  		  cc1.area.set_area(directory1.area*64/XML->sys.domain_size);
//
//	  		  cout<<"CC
// area="<<(cc.area.get_area()+cc1.area.get_area())*1e-6<<endl;
// cout<<"CC Power="<<cc.power.readOp.dynamic + cc1.power.readOp.dynamic <<endl;
//			  ccTot.area.set_area((cc.area.get_area()+cc1.area.get_area())*1e-6);
//			  ccTot.power = cc.power + cc1.power;
//	  	  }
//	  	  else if (XML->sys.first_level_dir==1)
//	  	  {
//	  		  inv_dir.maxPower =
// inv_dir.caches.local_result.power.searchOp.dynamic*clockRate*XML->sys.domain_size;
//	  		  cc1.power.readOp.dynamic =
// inv_dir.maxPower*scktRatio*(64/XML->sys.domain_size);
// cc1.power.readOp.leakage
//=
// inv_dir.caches.local_result.power.readOp.leakage*inv_dir.caches.l_ip.nbanks*XML->sys.domain_size;
//
//	  		  cc1.area.set_area(inv_dir.area*64/XML->sys.domain_size);
//			  cout<<"CC
// area="<<(cc.area.get_area()+cc1.area.get_area())*1e-6<<endl;
// cout<<"CC Power="<<cc.power.readOp.dynamic + cc1.power.readOp.dynamic <<endl;
//			  ccTot.area.set_area((cc.area.get_area()+cc1.area.get_area())*1e-6);
//			  ccTot.power = cc.power + cc1.power;
//
//	  	  }
//	  	  else if (XML->sys.first_level_dir==2)
//	  	  {
//			  cout<<"CC area="<<cc.area.get_area()*1e-6<<endl;
//			  cout<<"CC Power="<<cc.power.readOp.dynamic<<endl;
//			  ccTot.area.set_area(cc.area.get_area()*1e-6);
//			  ccTot.power = cc.power;
//	  	  }
//  }
//
// cout<<"L2cache size="<<L2Tot.area.get_area()*1e-6<<endl;
// cout<<"L2cache dynamic power="<<L2Tot.power.readOp.dynamic<<endl;
// cout<<"L2cache laeakge power="<<L2Tot.power.readOp.leakage<<endl;
//
//  ///cout<<"llCache.maxPower=" <<llCache.maxPower<<endl;
//
//
//  maxPower          +=  llCache.maxPower;
//  ///cout<<"maxpower=" <<maxPower<<endl;
//
////  maxPower	  +=  pipeLogicCache.power.readOp.dynamic*clockRate;
////
//////cout<<"pipeLogic.power="<<pipeLogicCache.power.readOp.dynamic*clockRate<<endl;
////  ///cout<<"maxpower=" <<maxPower<<endl;
////
////  maxPower	  +=  pipeLogicDirectory.power.readOp.dynamic*clockRate;
////
//////cout<<"pipeLogic.power="<<pipeLogicDirectory.power.readOp.dynamic*clockRate<<endl;
////  ///cout<<"maxpower=" <<maxPower<<endl;
////
////  //clock power
////  maxPower += clockNetwork.total_power.readOp.dynamic*clockRate;
////
//////cout<<"clockNetwork.total_power="<<clockNetwork.total_power.readOp.dynamic*clockRate<<endl;
////  ///cout<<"maxpower=" <<maxPower<<endl;
//
//}
