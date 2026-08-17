// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2026 Advanced Micro Devices, Inc. All rights reserved

#ifndef AIE_DTRACE_UTIL_DOT_H
#define AIE_DTRACE_UTIL_DOT_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

extern "C" {
#include <aie_codegen.h>
}

namespace xdp::aie::dtrace {

  // Shim bandwidth metric sets used for Debug.aie_dtrace (not part of standard aie_profile ini).
  std::map<std::string, std::vector<XAie_Events>> getBandwidthInterfaceTileEventSets(int hwGen);

  // One PLIO controller entry parsed from plip_info.json.
  struct PlioInfoPort {
    uint8_t column;    // Shim (interface tile) column that hosts the PLIO controller
    uint8_t channel;   // south<N> port number (channel 0 == south0, channel 1 == south1, ...)
  };

  // True for the plio_read_bandwidth / plio_write_bandwidth metric sets.
  bool isPlioBandwidthMetric(const std::string& metricSet);

  // AIE2PS shim stream-switch port index for a south<channel> PLIO port.
  // Per the AIE2 shim slave/master port table, south0 maps to physical port 2,
  // south1 to 3, etc. (tile_control_rsp=0, fifo=1 occupy the first two slots).
  uint8_t getPlioSouthStreamPortIndex(uint8_t channel);

  // Parse the PLIO overlay metadata (plip_info.json) and return the available
  // PLIO controller ports (column + south channel). Reads from a hardcoded path
  // for now; see the implementation for the TODO to source the final path.
  std::vector<PlioInfoPort> parsePlioInfoJson();

} // namespace xdp::aie::dtrace

#endif
