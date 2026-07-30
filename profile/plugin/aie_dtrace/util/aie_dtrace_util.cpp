// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2026 Advanced Micro Devices, Inc. All rights reserved

#define XDP_PLUGIN_SOURCE

#include "xdp/profile/plugin/aie_dtrace/util/aie_dtrace_util.h"

namespace xdp::aie::dtrace {

  std::map<std::string, std::vector<XAie_Events>>
  getBandwidthInterfaceTileEventSets(int hwGen)
  {
    (void)hwGen;
    return {
      {"read_bandwidth", {XAIE_EVENT_PORT_RUNNING_0_PL, XAIE_EVENT_PORT_RUNNING_1_PL}},
      {"write_bandwidth", {XAIE_EVENT_PORT_RUNNING_0_PL, XAIE_EVENT_PORT_RUNNING_1_PL}},
      {"ddr_bandwidth",
       {XAIE_EVENT_PORT_RUNNING_0_PL, XAIE_EVENT_PORT_RUNNING_1_PL, XAIE_EVENT_PORT_RUNNING_2_PL,
        XAIE_EVENT_PORT_RUNNING_3_PL}},
      {"peak_read_bandwidth",
       {XAIE_EVENT_PORT_RUNNING_0_PL, XAIE_EVENT_PORT_STALLED_0_PL,
        XAIE_EVENT_PORT_RUNNING_1_PL, XAIE_EVENT_PORT_STALLED_1_PL}},
      {"peak_write_bandwidth",
       {XAIE_EVENT_PORT_RUNNING_0_PL, XAIE_EVENT_PORT_STALLED_0_PL,
        XAIE_EVENT_PORT_RUNNING_1_PL, XAIE_EVENT_PORT_STALLED_1_PL}},
    };
  }

  bool isDetailedBandwidthMetric(const std::string& metricSet)
  {
    return (metricSet == "detailed_ddr_read_bandwidth") ||
           (metricSet == "detailed_ddr_write_bandwidth");
  }

  bool isPeakBandwidthMetric(const std::string& metricSet)
  {
    return (metricSet == "peak_read_bandwidth") || (metricSet == "peak_write_bandwidth");
  }

  uint8_t getVe2DmaPortIndex(bool isMaster, uint8_t channel)
  {
    if (isMaster)
      return (channel == 0) ? 3 : 5;
    return (channel == 0) ? 5 : 9;
  }

  namespace {

    bool wantsReadChannels(const std::string& metricSet)
    {
      return (metricSet == "read_bandwidth") ||
             (metricSet == "peak_read_bandwidth") ||
             (metricSet == "detailed_ddr_read_bandwidth");
    }

    bool wantsWriteChannels(const std::string& metricSet)
    {
      return (metricSet == "write_bandwidth") ||
             (metricSet == "peak_write_bandwidth") ||
             (metricSet == "detailed_ddr_write_bandwidth");
    }

    bool isGmioChannelUsed(const tile_type& tile, bool isMaster, uint8_t channel)
    {
      static constexpr uint8_t NUM_DMA_CHANNELS = 2;
      if (channel >= NUM_DMA_CHANNELS)
        return false;

      const auto& names = isMaster ? tile.s2mm_names : tile.mm2s_names;
      if (channel >= names.size())
        return false;

      return names[channel] != "unused";
    }

  } // namespace

  std::vector<GmioDmaChannel>
  getUsedGmioDmaChannels(const tile_type& tile, const std::string& metricSet,
                         uint8_t specifiedChannel)
  {
    std::vector<GmioDmaChannel> channels;

    auto tryAdd = [&](bool isMaster, uint8_t ch) {
      if (isGmioChannelUsed(tile, isMaster, ch))
        channels.push_back({ch, isMaster});
    };

    if (isDetailedBandwidthMetric(metricSet)) {
      bool isWrite = (metricSet == "detailed_ddr_write_bandwidth");
      tryAdd(isWrite, (specifiedChannel <= 1) ? specifiedChannel : 0);
      return channels;
    }

    if (wantsReadChannels(metricSet) || metricSet == "ddr_bandwidth") {
      for (uint8_t ch = 0; ch < 2; ++ch)
        tryAdd(false, ch);
    }

    if (wantsWriteChannels(metricSet) || metricSet == "ddr_bandwidth") {
      for (uint8_t ch = 0; ch < 2; ++ch)
        tryAdd(true, ch);
    }

    return channels;
  }

  std::vector<PlioStreamPort>
  getUsedPlioStreamPorts(const tile_type& tile, const std::string& metricSet)
  {
    std::vector<PlioStreamPort> ports;

    if (!isPeakBandwidthMetric(metricSet))
      return ports;

    for (size_t i = 0; i < tile.stream_ids.size(); ++i) {
      if (i >= tile.is_master_vec.size())
        break;

      bool isMaster = tile.is_master_vec[i] != 0;
      // peak_read = slave/input PLIO; peak_write = master/output PLIO
      if ((metricSet == "peak_read_bandwidth") && isMaster)
        continue;
      if ((metricSet == "peak_write_bandwidth") && !isMaster)
        continue;

      uint8_t streamId = tile.stream_ids[i];
      if (streamId < tile.port_names.size() && tile.port_names[streamId] == "unused")
        continue;

      ports.push_back({streamId, isMaster});
    }

    return ports;
  }

} // namespace xdp::aie::dtrace
