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
#ifndef __FUNCTIONAL_UNIT_H__
#define __FUNCTIONAL_UNIT_H__

#include "XML_Parse.h"
#include "arch_const.h"
#include "basic_circuit.h"
#include "basic_components.h"
#include "cacti_interface.h"
#include "component.h"
#include "const.h"
#include "decoder.h"
#include "parameter.h"
#include "xmlParser.h"

#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/utility.hpp>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>

class FunctionalUnit : public Component {
public:
  int ithCore;
  InputParameter interface_ip;
  CoreDynParam coredynp;
  double FU_height;
  double clockRate;
  double executionTime;
  double num_fu;
  double energy;
  double base_energy;
  double per_access_energy;
  double leakage;
  double gate_leakage;
  bool is_default;
  enum FU_type fu_type;
  statsDef tdp_stats;
  statsDef rtp_stats;
  statsDef stats_t;
  powerDef power_t;

  FunctionalUnit();
  void set_params(const ParseXML *XML_interface,
                  int ithCore_,
                  InputParameter *interface_ip_,
                  const CoreDynParam &dyn_p_,
                  enum FU_type fu_type);
  void set_stats(const ParseXML *XML);
  void computeArea();
  void computePower();
  void computeRuntimeDynamicPower();
  void display(uint32_t indent = 0, bool enable = true);
  void leakage_feedback(double temperature);

private:
  bool init_params;
  bool init_stats;
  bool set_area;
  bool long_channel;
  bool power_gating;
  bool embedded;

  // Power:
  double area_t;

  // Stats:
  unsigned int mul_accesses;
  unsigned int ialu_accesses;
  unsigned int fpu_accesses;

  // Private Methods:
  void computeLeakage();

  // Serialization
  friend class boost::serialization::access;

  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &area_t;
    ar &set_area;
    Component::serialize(ar, version);
  }
};

#endif // __FUNCTIONAL_UNIT_H__
