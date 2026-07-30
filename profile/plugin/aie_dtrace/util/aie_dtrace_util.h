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

#include "xdp/profile/database/static_info/aie_constructs.h"

namespace xdp::aie::dtrace {

  std::map<std::string, std::vector<XAie_Events>> getBandwidthInterfaceTileEventSets(int hwGen);

  struct GmioDmaChannel {
    uint8_t channel;   // DMA channel number (0 or 1)
    bool isMaster;     // true = S2MM (output), false = MM2S (input)
  };

  struct PlioStreamPort {
    uint8_t streamId;  // SOUTH stream-switch port id from metadata
    bool isMaster;     // false = input (slave), true = output (master)
  };

  bool isDetailedBandwidthMetric(const std::string& metricSet);
  bool isPeakBandwidthMetric(const std::string& metricSet);

  // VE2 NoC0 DMA stream-switch port index for a GMIO DMA channel.
  uint8_t getVe2DmaPortIndex(bool isMaster, uint8_t channel);

  // GMIO DMA channels in use for the metric set (from mm2s/s2mm names in metadata).
  std::vector<GmioDmaChannel> getUsedGmioDmaChannels(const tile_type& tile,
                                                    const std::string& metricSet,
                                                    uint8_t specifiedChannel);

  // PLIO SOUTH stream ports in use for peak_read_bandwidth / peak_write_bandwidth.
  std::vector<PlioStreamPort> getUsedPlioStreamPorts(const tile_type& tile,
                                                     const std::string& metricSet);

} // namespace xdp::aie::dtrace

#endif
