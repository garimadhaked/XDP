// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2026 Advanced Micro Devices, Inc. All rights reserved

#define XDP_PLUGIN_SOURCE

#include "xdp/profile/plugin/aie_dtrace/util/aie_dtrace_util.h"

#include <filesystem>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "core/common/message.h"

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

  bool isPlioBandwidthMetric(const std::string& metricSet)
  {
    return (metricSet == "plio_read_bandwidth") || (metricSet == "plio_write_bandwidth");
  }

  uint8_t getPlioSouthStreamPortIndex(uint8_t channel)
  {
    // south0 -> physical stream-switch port 2 (slots 0/1 are tile_control/fifo).
    static constexpr uint8_t SOUTH0_STREAM_PORT_INDEX = 2;
    return static_cast<uint8_t>(SOUTH0_STREAM_PORT_INDEX + channel);
  }

  std::vector<PlioInfoPort> parsePlioInfoJson()
  {
    std::vector<PlioInfoPort> ports;

    // TODO: The location of the PLIO overlay metadata is not finalized yet. For
    // now we hardcode the example overlay's plip_info.json. Replace this with the
    // real overlay path (resolved at runtime) once it is available.
    const std::filesystem::path jsonPath =
        "/public/bugcases/CR/1269000-1269999/1269377/wts_prefetch/"
        "2026.1.dbg/with-profiling/overlay/plip_info.json";

    std::error_code ec;
    if (!std::filesystem::exists(jsonPath, ec)) {
      xrt_core::message::send(xrt_core::message::severity_level::debug, "XRT",
          "AIE dtrace: PLIO metadata file not found: " + jsonPath.string());
      return ports;
    }

    try {
      boost::property_tree::ptree pt;
      boost::property_tree::read_json(jsonPath.string(), pt);

      auto pliosItr = pt.get_child_optional("plios");
      if (!pliosItr) {
        xrt_core::message::send(xrt_core::message::severity_level::warning, "XRT",
            "AIE dtrace: 'plios' section missing in " + jsonPath.string());
        return ports;
      }

      for (const auto& plio : *pliosItr) {
        const auto& node = plio.second;
        auto colStr = node.get_optional<std::string>("column");
        auto chStr  = node.get_optional<std::string>("channel");
        if (!colStr)
          continue;

        PlioInfoPort port;
        port.column  = static_cast<uint8_t>(std::stoi(*colStr));
        port.channel = chStr ? static_cast<uint8_t>(std::stoi(*chStr)) : 0;
        ports.push_back(port);
      }
    }
    catch (const std::exception& e) {
      xrt_core::message::send(xrt_core::message::severity_level::warning, "XRT",
          std::string("AIE dtrace: failed to parse PLIO metadata: ") + e.what());
      ports.clear();
    }

    return ports;
  }

} // namespace xdp::aie::dtrace
