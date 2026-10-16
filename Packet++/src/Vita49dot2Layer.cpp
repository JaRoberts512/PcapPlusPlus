#define LOG_MODULE PacketLogModuleVita49dot2Layer

#include <unordered_map>

#include "Vita49dot2Layer.h"
#include "GeneralUtils.h"
#include "PayloadLayer.h"

namespace pcpp
{
    // This unordered map provides human-readable descriptions for each version of 
    // VITA49.2 packet types as defined in table 5.1.1-1 of ANSI/VITA49.2-2017 specification. 
	static const std::unordered_map<V49_PACKET_TYPE, std::string> Vita49dot2EnumToStringProtocolVersion
    {
		{ V49_PACKET_TYPE::DATA,                "Signal Data Packet without Stream Identifier"          },
		{ V49_PACKET_TYPE::DATA_STREAM_ID,      "Signal Data Packet with Stream Identifier"             },
		{ V49_PACKET_TYPE::EXT_DATA,            "Extension Data Packet without Stream Identifier"       },
		{ V49_PACKET_TYPE::EXT_DATA_STREAM_ID,  "Extension Data Packet with Stream Identifier"          },
		{ V49_PACKET_TYPE::CONTEXT,             "Context Packet"                                        },
		{ V49_PACKET_TYPE::EXT_CONTEXT,         "Extension Context packet"                              },
        { V49_PACKET_TYPE::COMMAND,             "Command Packet"                                        },
		{ V49_PACKET_TYPE::EXT_COMMAND,         "Extension Command Packet"                              },
        { V49_PACKET_TYPE::RESERVED,            "Reserved for future VRT Packet Types"                  },
	};

    Vita49dot2Layer::Vita49dot2Layer(uint8_t* data, size_t dataLen, Layer* prevLayer, Packet* packet)
	    : Layer(data, dataLen, prevLayer, packet, DOIP)
	{

    }

    V49_PACKET_TYPE Vita49dot2Layer::getPacketType() const
    {
        V49_PACKET_TYPE version = static_cast<V49_PACKET_TYPE>(getVrtHeader()->packetType);

        switch (version)
        {
            case V49_PACKET_TYPE::DATA:
            case V49_PACKET_TYPE::DATA_STREAM_ID:
            case V49_PACKET_TYPE::EXT_DATA:
            case V49_PACKET_TYPE::EXT_DATA_STREAM_ID:
            case V49_PACKET_TYPE::CONTEXT:
            case V49_PACKET_TYPE::EXT_CONTEXT:
            case V49_PACKET_TYPE::COMMAND:
            case V49_PACKET_TYPE::EXT_COMMAND:
                return static_cast<V49_PACKET_TYPE>(version);
            
            default:
                return V49_PACKET_TYPE::RESERVED;
        }
    }

    std::string Vita49dot2Layer::getPacketTypeAsString() const
    {
        return Vita49dot2EnumToStringProtocolVersion.find(getPacketType())->second;
    }

    size_t Vita49dot2Layer::getPrologueLength() const 
    {
        size_t len;

        if (hasStreamId())
            len += sizeof(std::array<uint8_t, VRT_STREAMID_LEN>);
        if (hasClassId())
            len += sizeof(std::array<uint8_t, VRT_CLASSID_LEN>);
        if (hasIntTimestamp())
            len += sizeof(std::array<uint8_t, VRT_INT_SECONDS_TIMESTAMP_LEN>);
        if (hasFracTimestamp())
            len += sizeof(std::array<uint8_t, VRT_FRAC_SECONDS_TIMESTAMP_LEN>);
        
        return len;
    }

    void Vita49dot2Layer::setStreamId(std::array<uint8_t, VRT_STREAMID_LEN> streamId)
    {
        if(!hasStreamId())
        {
            PCPP_LOG_DEBUG("[Invalid VITA49.2 function call]: Packet type incompatable with Stream ID inclusion.");
            return;
        }

        extendLayer(streamId.size(), sizeof(streamId));

        /// TODO: Add rest of setter func.

        containsStreamId = true;
    }

    void Vita49dot2Layer::setClassId(std::array<uint8_t, VRT_CLASSID_LEN> classId) 
    {
        if(!hasClassId())
        {
            PCPP_LOG_DEBUG("[Invalid VITA49.2 function call]: Class ID field not enabled in packet header.");
            return;
        }

        extendLayer(classId.size(), sizeof(classId));
        
        /// TODO: Add rest of setter func.

        containsClassId = true;
    }

    void  Vita49dot2Layer::clearClassId()
    {
        if(!hasClassId())
        {
            PCPP_LOG_DEBUG("[Invalid VITA49.2 function call]: Class ID field not enabled in packet header.");
            return;
        }

        shortenLayer(getPrologueLength(), sizeof(std::array<uint8_t, VRT_CLASSID_LEN>));

        containsClassID = false;
    }

}
