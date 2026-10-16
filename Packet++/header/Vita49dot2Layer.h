#pragma once

#include <cstring>
#include <vector>

#include "Layer.h"
#include "Logger.h"

///
/// @file Vita49dot2.h
///
/// This file contains classes for parsing, creating, and editing VITA49.2 packets. 
/// # Definitions and Terminology
/// - 'VRT' Is an acronym used in documentation throughout this file which stands for 'VITA Radio Transport' and is used
/// in this instance to refer specifically to the VITA49.2 standard, 'VRT' and 'VITA49.2' are interchangeable. 
/// - 'Data' May refer to signal data, extention data, or both. 
/// - 'VRT Streams' refers to the emission of a sequence of packets from a specific source in a VRT enabled system. 
/// usually, at the very least, a VRT Stream will include Data Packets, with Context Packets interspersed throughout, 
/// there is no rule or guide for the frequency of Context or Command Packets emitted in a stream, but they are usually
/// emmited at regular intervals, for example, one in every 1000 Data Packets. 

/// @namespace pcpp
/// @brief The main namespace for the PcapPlusPlus lib.
namespace pcpp
{

    /// @struct vrthdr
    /// VRT packet header structure. 
#pragma pack(push, 1)
    typedef struct vrthdr
    {
        /// Packet type.
        V49_PACKET_TYPE packetType : 4;
        /// Determines whether the packet prologue includes class ID.
        unsigned hasClassId : 1;
        /// Some Indicators (dependant on packet type).
        unsigned typeSpecificIndicator1 : 1;
        unsigned typeSpecificIndicator2 : 1;
        unsigned typeSpecificIndicator3 : 1;
        /// Integer timestamp codes. 
        TSI tsi : 2;
        /// Fractional timestamp codes. 
        TSF tsf : 2;
        /// Packet number in stream. Rolls over from `1111` to `0000` as count increments. 
        unsigned packetCount : 4;
        /// Size of packet in number of 32 bit words. Including header, prologue, payload, and any optional fields. 
        unsigned packetSize : 16;
    } vrthdr;
#pragma pack(pop)
    static_assert(sizeof(vrthdr) == 4, "VRT header must be a single 32 bit word.");

    namespace Vita49dot2Constants
    {
        /// @brief Length (in bytes) of the Stream ID field.
        static constexpr size_t VRT_STREAMID_LEN = 4;

        /// @brief Length (in bytes) of the Class ID field.
        static constexpr size_t VRT_CLASSID_LEN = 4;

        /// @brief Length (in bytes) of the Integer Seconds timestamp.
        static constexpr size_t VRT_INT_SECONDS_TIMESTAMP_LEN = 4;

        /// @brief Length (in bytes) of the Fractional Seconds timestamp.
        static constexpr size_t VRT_FRAC_SECONDS_TIMESTAMP_LEN = 8;

        /// @brief Length (in bytes) of the Controller/Controllee ID field.
        static constexpr size_t VRT_CTRL_ID_LEN = 16;

        /// @brief Length (in bytes) of the Controller/Controllee UUID field.
        static constexpr size_t VRT_CTRL_UUID_LEN = 16;

    } // namespace Vita49dot2Constants

    /// Packet type indicator.
    enum class V49_PACKET_TYPE : unsigned int
    {   
        /// Signal Data Packet without stream Identifier.
        DATA                = 0b0000,
        /// Signal Data Packet with stream Identifier.
        DATA_STREAM_ID      = 0b0001, 
        /// Signal Data Packet without stream Identifier.
        EXT_DATA            = 0b0010,
        /// Signal Data Packet with stream Identifier.
        EXT_DATA_STREAM_ID  = 0b0011,
        /// Context packet.
        CONTEXT             = 0b0100,
        /// Extension context packet.
        EXT_CONTEXT         = 0b0101,
        /// Command packet.
        COMMAND             = 0b0110,
        /// Extension command packet.
        EXT_COMMAND         = 0b0111,
        /// Remaining values reserved.
        RESERVED            = 0b1111,
    };

    /// Timestamp integer type. 
    enum class TSI : unsigned int 
    {
        NONE    = 0b00,
        UTC     = 0b01,
        GPS     = 0b10,
        OTHER   = 0b11,
    };

    /// Timestamp fractional type.
    enum class TSF : unsigned int 
    {
        NONE                = 0b00,
        SAMPLE_COUNT        = 0b01,
        REAL_TIME           = 0b10,
        FREE_RUNNNING_COUNT = 0b11, 
    };

    /// VITA49 Compliance specification codes. 
    enum class VITA49_COMPLIANCE_NO : unsigned int
    {
        V49 = 0x00000001,
        V49_DOT_1 = 0x0000002,
        V49_A = 0x0000003,
        V49_DOT_2 = 0x0000004,
    };

    /// Defines type of spectrum data in spectrum Data Packets.
    enum class SPECTRUM_TYPE : unsigned int  
    {
        /// Default "no setting" value.
        NO_SETTING  = 0b00000000,
        /// Log power (dB).
        LOG_POWER   = 0b00000001,
        /// Cartesian (I, Q).
        CARTESIAN   = 0b00000010,
        /// Polar (magnitude, phase).
        POLAR       = 0b00000011,
        /// Magnitude
        MAGNITUDE   = 0b00000100,
        /// bits 0b00000101 - 0b01111111 reserved.
        /// bits 0b01111111 - 0b11111111 user defined types. 
    };

    /// If multiple bits are set in the Averaging Type bit-field, creating a Sample Vector of multiple data items, the 
    /// rules set out in VITA 49.2-2017 Section 6.1.1.3 regarding Sample Vectors must apply to the Signal Spectral 
    /// Data stream.
    enum class SPECTRUM_AVERAGING_TYPE : unsigned int 
    {
        NO_AVERAGING = 0b00000000,
        LINEAR_AVERAGING = 0b00000001,
        PEAK_HOLD = 0b00000010,
        MIN_HOLD = 0b00000011,
        EXPONENTIAL_AVERAGING = 0b00001000,
        MEADIAN_AVERAGING = 0b00010000,
        SMOOTHING = 0b00100000,
        /// others... reserved
    };

    /// Spectrum window type. For different "alpha" coefficient values for a given window, these have been given explicit
    /// enumerations here following the window name. 
    enum class SPECTRUM_WINDOW_TYPE : unsigned int 
    {
        RECTANGLE = 0,
        TRIANGLE = 1,
        /// Hanning - alpha value 1
        HANNING_1 = 3,
        /// Hanning - alpha value 2
        HANNING_2 = 4,
        /// Hanning - alpha value 3
        HANNING_3 = 5,
        /// Hanning - alpha value 4
        HANNING_4 = 6,
        RIESZ = 7,
        RIEMANN = 8,
        DE_LA_VALLEPOUSSIN = 9,
        /// Tukey - alpha value 0.25
        TUKEY_0_dot_25 = 10,
        /// Tukey - alpha value 0.50
        TUKEY_0_dot_5 = 11,
        /// Tukey - alpha value 0.75
        TUKEY_0_dot_75 = 12,
        BOHMAN = 13,
        /// Poisson - alpha value 2
        POISSON_2 = 14,
        /// Poisson - alpha value 3
        POISSON_3 = 15,
        /// Poisson - alpha value 4
        POISSON_4 = 16,
        /// Hanning-Poisson - alpha value 0.5
        HANNING_POISSON_0_dot_5 = 17,
        /// Hanning-Poisson - alpha value 1
        HANNING_POISSON_1 = 18,
        /// Hanning-Poisson - alpha value 2
        HANNING_POISSON_2 = 19,
        /// Cauchy - alpha value 3
        CAUCHY_3 = 20,
        /// Cauchy - alpha value 4
        CAUCHY_4 = 21,
        /// Cauchy - alpha value 5
        CAUCHY_5 = 22,
        /// Gaussian - alpha value 2.5
        GAUSSIAN_2_dot_5 = 23,
        /// Gaussian - alpha value 3
        GAUSSIAN_3 = 24,
        /// Gaussian - alpha value 3.5
        GAUSSIAN_3_dot_5 = 25,
        /// Dolph-Chebyshev - alpha value 2.5
        DOLPH_CHEBYSHEV_2_dot_5 = 26,
        /// Dolph-Chebyshev - alpha value 3
        DOLPH_CHEBYSHEV_3 = 27,
        /// Dolph-Chebyshev - alpha value 3.5
        DOLPH_CHEBYSHEV_3_dot_5 = 28,
        /// Dolph-Chebyshev - alpha value 4
        DOLPH_CHEBYSHEV_4 = 29,
        /// Kaiser-Bessel - alpha value 2
        KAISER_BESSEL_2 = 30,
        /// Kaiser-Bessel - alpha value 2.5
        KAISER_BESSEL_2_dot_5 = 31,
        /// Kaiser-Bessel - alpha value 3
        KAISER_BESSEL_3 = 32,
        /// Kaiser-Bessel - alpha value 3.5
        KAISER_BESSEL_3_dot_5 = 33,
        /// Barcilon-Temes - alpha value 3
        BARCILON_TEMES_3 = 34,
        /// Barcilon-Temes - alpha value 3.5
        BARCILON_TEMES_3_dot_5 = 35,
        /// Barcilon-Temes - alpha value 4
        BARCILON_TEMES_4 = 36,
        EXACT_BLACKMAN = 37,
        BLACKMAN = 38,
        MINIMUM_3_SAMPLE_BLACKMAN_HARRIS = 39,
        MINIMUM_4_SAMPLE_BLACKMAN_HARRIS = 40,
        /// 61dB 3-Sample Blackmann-Harris
        MINIMUM_3_SAMPLE_BLACKMAN_HARRIS_61dB = 41,
        /// 74dB 3-Sample Blackmann-Harris
        MINIMUM_4_SAMPLE_BLACKMAN_HARRIS_74dB = 42,
        /// 4-Sample Kaiser-Bessel
        KAISER_BESSEL_4SAMPLE = 43,

        /// bits 44..99 reserved.
        /// bits 100..255 OTHER, specified in class definition.
    };

    enum class TSE_CODE : unsigned int
    {
        UNSPECIFIED = 0b00,
        UTC_EPOCH   = 0b01,
        GPS_EPOCH   = 0b10,
        POSIX_EPOCH = 0b11,
    };

    enum class LSH_CODE : unsigned int
    {
        LEAP_SECONDS_NA = 0b00,
        LEAP_SECONDS_NORMAL = 0b01,
        LEAP_SECONDS_VIA_DUPLICATION = 0b10,
        LEAP_SECONDS_VIA_OVERFLOW = 0b11,
    };

    enum class LSP_CODE : unsigned int
    {
        UNSPECIFIED = 0b00,
        CURRENT_DAY_HAS_86399_SECONDS = 0b01,
        CURRENT_DAY_HAS_86400_SECONDS = 0b10,
        CURRENT_DAY_HAS_86401_SECONDS = 0b11,
    };

    enum class TIME_SOURCE : unsigned int
    { 
        UNSPECIFIED = 0b000,
        ATOMIC_CLOCK = 0b001,
        SATALITE_SYSTEM = 0b010,
        TERRESTRIAL_RADIO =0b011,
        PTP = 0b100,
        NTP_OR_SNTP = 0b101,
        USER_DEFINED_6 = 0b110,
        USER_DEFINED_7 = 0b111,
    };

    enum class EMS_ORGANISATIONAL_RELATIONSHIP : unsigned int 
    {
        COALITION = 0b00,
        KNOWN = 0b01,
        UNKNOWN = 0b10,
        // RESERVED = 0b11,
    };

    enum class SEA_STATE_LEVELS : unsigned int {
        /// No swell
        CALM_GLASSY = 0b00000,
        /// Very low (short and low wave)
        CALM_RIPPLED = 0b00001,
        /// Low (long and low wave)
        SMOOTH = 0b00010,
        /// Light (short and moderate wave)
        SLIGHT = 0b00011,
        /// Moderate (average and moderate wave)
        MODERATE = 0b00100,
        /// Moderate rough (long and moderate wave)
        ROUGH = 0b00101,
        /// Rough (short and heavy wave)
        VERY_ROUGH = 0b00110,
        /// High (average and heavy wave)
        HIGH = 0b00111,
        /// Very High (long and heavy wave)
        VERY_HIGH = 0b01000,
        /// Confused (wave length and height indefinable)
        PHENOMENAL = 0b01001,
    };

    /// This enum is intended to be duplicated in the state_and_event_indicator_field struct, where for each indicator bit
    /// in position [31..20] there is a corresponding enable bit in position [19..8]. The time at which indicators are 
    /// valid is given by the timestamp of the context packet. 
    enum class STATE_AND_EVENT_INDICATOR_ENABLE : unsigned int
    { 
        /// - 1: Timestamps in the Context Packet Stream and in the associated Data Packet Stream are calibrated to some 
        /// external reference. 
        /// - 0: Timestamps are free-running and may be inaccurate.
        CALIBRATED_TIME = 0x800,
        /// - 1: Data in the associated Data Packet is valid. 
        /// - 0: Some condition exists that may invalidate the Data.
        VALID_DATA = 0x400,
        /// - 1: Any phase-locked loops (PLL) affecting the Described Signal are locked and stable. 
        /// - 0: At least one PLL is not locked and stable.
        REFERENCE_LOCK = 0x200,
        /// - 1: Automatic gain control.
        /// - 0: Manual gain control.
        AGC_MGC = 0x100,
        /// - 1: Described signal contains some detected signal.
        DETECTED_SIGNAL = 0x080,
        /// - 1: Spectrum of the signal conveyed in the data payload is inverted with respect to the signal at the 
        ///      Reference Point.
        SPECTRAL_INVERSION = 0x040,
        /// - 1: When `TSM` (TimeStamp Mode) `= 0`, indicates that the data sample in the paired Data Packet stream 
        ///      was over-range at the time given in the context packet timestamp. When `TSM = 1`, indicates that at least
        ///      data sample in the paired Data Packet with corresponding timestamp is over-range. Definition of 
        ///      'over-range' given in rule 7.1.5.11-1 of ANSI/VITA-49.2-2017.
        OVER_RANGE = 0x0200,
        /// - 1: When `TSM` (TimeStamp Mode) `= 0`, indicates that the paired Data Packet contains a data sample 
        ///      discontinuity. When `TSM = 1`, indicates that the paired data packet with corresponding timestamp
        ///      contains at least one data sample discontinuity. 
        SAMPLE_LOSS = 0x010,
    };

    /// Contains bit assignments to indicate prescribed action for a controllee. 
    enum class ACTION_MODES : unsigned int 
    {
        /// Controllee to implement commands and respond with appropriate acknowledge packets. 
        EXECUTE = 0b10,
        /// Commands the controllee to respond as if controlls were implemented, but to take no actual action.
        DRY_RUN = 0b01,
        /// Controllee takes no action, no fields provided in command packet.
        NO_ACTION = 0b00,
    };

    /// Specifies how packet’s controls are to be executed with respect to the timestamp.
    enum class TIMING_CTRL : unsigned int
    {   
        /// 111..101 Reserved.
        PERMITTED_EARLY_LATE_TIMING_EXECUTION = 0b100,
        EARLY_SPECIFIED_TIME_EXECUTION = 0b011,
        LATE_SPECIFIED_TIME_EXECUTION = 0b010,
        TIMING_PRECISION_DEVICE_EXECUTION_ONLY = 0b001,
        IGNORE_TIMESTAMP = 0b000,
    };

    enum class TIMING_ACK : unsigned int
    {   
        /// Did not execute some controls at the specified time.
        EXECUTION_TIMING_ISSUES = 0b111,
        /// 110..101 Reserved.
        /// Application timing execution window
        APPLICATION_TIMING_EXECUTION_WINDOW = 0b100,
        EARLY_SPECIFIED_TIME_EXECUTION = 0b011,
        LATE_SPECIFIED_TIME_EXECUTION = 0b010,
        DEVICE_PRECISION_TIMING_EXECUTION_WINDOW = 0b001,
        IGNORE_TIMESTAMP = 0b000,
    };

    enum class DATA_SAMPLE_TYPE : unsigned int 
    {
        REAL = 0b00,
        COMPLEX_CARTESIAN = 0b01,
        COMPLEX_POLAR = 0b10,
        /// RESERVED = 0b11,
    };

    enum class DATA_ITEM_FORMAT : unsigned int
    {
        /// 11111..11000 RESERVED
        UNSIGNED_FIXED_POINT_NON_NORMALISED = 0b10111,
        UNSIGNED_VRT_6BIT_EXP = 0b10110,
        UNSIGNED_VRT_5BIT_EXP = 0b10101,
        UNSIGNED_VRT_4BIT_EXP = 0b10100,
        UNSIGNED_VRT_3BIT_EXP = 0b10011,
        UNSIGNED_VRT_2BIT_EXP = 0b10010,
        UNSIGNED_VRT_1BIT_EXP = 0b10001,
        UNSIGNED_FIXED_POINT = 0b10000,
        IEEE_754_DOUBLE_PRECISION_FLOATING_POINT = 0b01111,
        IEEE_754_SINGLE_PRECISION_FLOATING_POINT = 0b01110,
        IEEE_754_HALF_PRECISION_FLOATING_POINT = 0b01101,
        /// 01100..01000 RESERVED
        SIGNED_FIXED_POINT_NON_NORMALISED = 0b00000,
        SIGNED_VRT_6BIT_EXP = 0b00001,
        SIGNED_VRT_5BIT_EXP = 0b00010,
        SIGNED_VRT_4BIT_EXP = 0b00011,
        SIGNED_VRT_3BIT_EXP = 0b00100,
        SIGNED_VRT_2BIT_EXP = 0b00101,
        SIGNED_VRT_1BIT_EXP = 0b00110,
        SIGNED_FIXED_POINT = 0b00111,
    };

    enum class VITA49_PORTS : uint16_t
    {
        /// TCP and UDP vita Ports.
        /// Standard port for VRT stream.
        TCP_UDP_PORT = 4991U,
    };

    /// @struct array_of_records_template
    /// Contains template for header of array-of-records type, defines structure for rest of array-of-records.
    typedef struct array_of_records_template_header
    {
        unsigned sizeOfArray : 32;
        unsigned headerSize : 8;
        unsigned numWordsOrRecord : 12;
        unsigned numRecords : 12; 
    } array_of_records_template_header;

    /// @struct class_id
    /// Class ID words stucture, part of VRT packet prologue, conditinally included.
#pragma pack(push, 1)
    typedef struct class_id
    {
        /// Number of pad bits.
        unsigned padCount : 5;
        /// Reserved bits. 
        unsigned _reserved : 3;
        /// Organisationally Unique Identifier.
        unsigned oui : 24;
        /// Information Class Code.
        unsigned icc : 16;
        /// Packet Class Code.
        unsigned pcc : 16;
    } class_id;
#pragma pack(pop)
    static_assert(sizeof(class_id) == 8, "ClassID size is not a single 32 bit word");

    /// @struct fractional_timestamp_part
    /// Fractional part of timestamp, if included, consinsts of two 32 bit words. 
#pragma pack(push, 1)
    typedef struct fractional_timestamp
    {
        /// Most significant bits. 
        unsigned msb : 32;
        /// Least significant bits. 
        unsigned lsb : 32;
    } fractional_timestamp;
#pragma pack(pop)
    static_assert(sizeof(fractional_timestamp) == 8, "Fractional part of timestamp is not 64 bits.");

    /// @struct trailer
    /// packet trailer. 32 bit trailer word, when set, is appended to the end of a packet.
#pragma pack(push, 1)
    typedef struct trailer
    {
        STATE_AND_EVENT_INDICATOR_ENABLE enables : 12;
        STATE_AND_EVENT_INDICATOR_ENABLE stateAndEventIndicators : 12;
        unsigned E : 1;
        unsigned associatedContextPacketCount : 7;
    } trailer;
#pragma pack(pop)
    static_assert(sizeof(trailer) == 4, "Trailer is not a single 32 bit word");

    /// @struct spectrum_type_subfield
    /// Subfield for spectrum field. 32 bit word containing basic characteristics of spectral data. 
#pragma pack(push, 1)
    typedef struct spectrum_type_subfield
    {
        unsigned _reserved_bits_ : 12;
        unsigned windowTime : 4;
        SPECTRUM_AVERAGING_TYPE averagingType : 8;
        SPECTRUM_TYPE spectrumType : 8;
    } spectrum_type_subfield;
#pragma pack(pop)
    static_assert(sizeof(spectrum_type_subfield) == 4, "Spectrum type is not a single 32 bit word");

    /// @struct window_type_subfield
    /// Subfield for spectrum field. 32 bit word describing window type of spectrum data when context field is enabled. 
#pragma pack(push, 1)
    typedef struct window_type_subfield
    {
        unsigned __reserved_bits_ : 16;
        unsigned __more_reserved_bits : 8;
        SPECTRUM_WINDOW_TYPE windowType : 8; 
    } window_type_subfield;
#pragma pack(pop)
    static_assert(sizeof(window_type_subfield) == 4, "Window-type subfield is not a single 32 bit word");

    /// @struct sectorStepScanCIW
    /// This context indicator word lists the available subfields that may be used in the Sector/Step-Scan field records.
    /// Only two of the listed sub-fields are REQUIRED in all cases. Others are optional. 
    ///
    /// Fields consist of 1 word unless specified otherwise. 
#pragma pack(push, 1)
    typedef struct sector_step_scan_CIW
    {
        /// Bit 31. Required. 
        unsigned sectorNumber : 1;
        /// Bit 30. Required. 2 words. 
        unsigned f1StartFreq : 1;
        /// Bit 29. Optional. 2 words. 
        unsigned f2StopFreq : 1;
        /// Bit 28. Optional. 2 words. 
        unsigned resolutionBandwidth : 1;
        /// Bit 27. Optional. 2 words. 
        unsigned tuneStepSize : 1;
        /// Bit 26. Optional. 
        unsigned numberOfPoints : 1;
        /// Bit 25. Optional. 
        unsigned defaultGain : 1;
        /// Bit 24. Optional. Two 16 bit values.
        unsigned threshold : 1;
        /// Bit 23. Optional. Two 16 bit values.
        unsigned dwellTime : 1;
        /// Bit 22. Optional. 
        unsigned startTime : 1;
        /// Bit 21. Optional. 
        unsigned time3 : 1;
        /// Bit 20. Optional. 
        unsigned time4 : 1;
        /// Bits 19..0 reserved bits. 
        unsigned __reserved_bits : 19;
    } sectorStepScanCIW;
#pragma pack(pop)
    static_assert(sizeof(sectorStepScanCIW) == 4, "CIW size is not a single 32 bit word.");

    /// @struct timestamp_details
#pragma pack(push, 1)
    typedef struct timestamp_details
    { 
        /// Bits 31..24. User defined bits. Bit 
        unsigned userDefinedBits : 8;
        /// Bits 23..19 reserved.
        unsigned _reserved_bits1 : 5;
        /// Bit 18. Indicates whether timestamp details apply to all packets in VRT stream. Set to `0` if timestamp 
        /// details only apply to context and paired data stream. 
        unsigned g : 1;
        /// Bits 17..16. Indicates type of timestamp epoch. See TSE_CODE enum.
        TSE_CODE tseCode : 2;
        /// Bits 15..14. Indicates the means by which leap seconds are handled in the packet timestamp. See LSH_CODE enum.
        LSH_CODE lshCode : 2;
        /// Bits 13..12. Indicates the number of seconds in the current day denoted by the packet timestamps. See LSP_CODE
        /// enum.
        LSP_CODE lspCode : 2;
        /// Bits 11..9. Indicates the timestamp source being used. See TIME_SOURCE enum. 
        TIME_SOURCE timeSource : 3;
        /// Bit 8. If `1`, conveys the difference in seconds between UTC time and POSIX time, as a signed twos-complement 
        /// number, and it represents the current total leap seconds count. If `0`, the value of the Current POSIX Time 
        /// Offset is undefined
        unsigned e : 1;
        /// Bits 7..0. Derived from the time signal from a GPS satellite or a PTP time source and can greatly assist 
        /// receivers of the packets that need to convert between POSIX and UTC time
        unsigned currentPOSIXTimeOffset : 8;
        /// ---
        /// **Second Word**
        /// Bits 32..0. Unsigned int value specifying the number of whole seconds, in the epoch denoted by the 
        /// TSE Code, describing the start of the epoch used for timestamps in all packets to which the Timestamp Details 
        /// field applies
        unsigned timestampEpoch : 32;
    } timestamp_details;
#pragma pack(pop)
    static_assert(sizeof(timestamp_details) == 8, "Timestamp details is not 64 bits.");

    /// @struct device_identifier_field
#pragma pack(push, 1)
    typedef struct device_identifier_field 
    {
        unsigned _reserved_bits1 : 8;
        /// IEEE Registered organisationally unique identifier in the range 00-00-00 to FF-FE-FF inclusive.
        unsigned manufacturerOUI : 24;
        unsigned _reserved_bits2 : 16;
        /// Identifier for model of device, specified by manufacturer. 
        unsigned deviceCode : 16;
    } device_identifier_field;
#pragma pack(pop)
    static_assert(sizeof(device_identifier_field) == 8, "Device ID is not 64 bits.");

    /// @struct version_information_field
#pragma pack(push, 1)
    typedef struct version_information_field
    {
        unsigned year : 6;
        unsigned day : 8;
        unsigned revision : 6;
        unsigned userDefined: 10;
    } version_information_field;
#pragma pack(pop)
    static_assert(sizeof(version_information_field) == 4, "Version information field is not a single 32 bit word.");

    /// @struct buffer_size_field
#pragma pack(push, 1)
    typedef struct buffer_size_field
    {
        unsigned bufferSize : 32; 
        unsigned _reserved_bits : 16;
        unsigned level : 8;
        unsigned status : 8;
    } buffer_size_field;
#pragma pack(pop)
    static_assert(sizeof(buffer_size_field) == 8, "Buffer size field is not 64 bits.");

    /// @struct state_and_event_indicator_field
#pragma pack(push, 1)
    typedef struct state_and_event_indicator_field
    {
        STATE_AND_EVENT_INDICATOR_ENABLE enables : 12;
        STATE_AND_EVENT_INDICATOR_ENABLE indicators : 12;
        unsigned userDefined : 8;
    } state_and_event_indicator_field;
#pragma pack(pop)
    static_assert(sizeof(state_and_event_indicator_field) == 4, "State and event indicator field is not a single 32 bit word.");

    /// @struct ecef_ephemeris_field
#pragma pack(push, 1)
    typedef struct ecef_ephemeris_field
    {
        unsigned _reserved_bits : 4;
        TSI tsi : 2;
        TSF tsf : 2; 
        unsigned gpsInsManufacturerOUI : 24;
        /// Timestamp in ECEF Ephemeris field and Formatted GPS/INS location field is the same format as the timestamp in 
        /// the VRT packet header, except the `00` value code means the timestamp words are present, but not specified. 
        unsigned integerSecondTimestamp : 32;
        unsigned fractionalSecondTimestamp1 : 32;
        unsigned fractionalSecondTimestamp2 : 32;
        /// Position words use 32 bit, two's compliment fixed point format with radix point between bits 5 and 4. 
        unsigned posiX : 32;
        unsigned posiY : 32;
        unsigned posiZ : 32;
        /// Position words use 32 bit, two's compliment fixed point format with radix point between bits 22 and 21. 
        unsigned altAlpha : 32;
        unsigned altBeta : 32;
        unsigned altPhi : 32;
        /// Velocity words use 32 bit, two's compliment fixed point format with radix point between bits 16 and 15. 
        unsigned velocitydx : 32;
        unsigned velocitydy : 32;
        unsigned velocitydz : 32;
    } ecef_ephemeris_field;
#pragma pack(pop)
    static_assert(sizeof(ecef_ephemeris_field) == 52, "ECEF Ephemeris field is not 416 bits.");

    /// @struct formatted_gps_ins_field
#pragma pack(push, 1)
    typedef struct formatted_gps_ins_field
    {
        unsigned _reserved_bits : 4;
        TSI tsi : 2;
        TSF tsf : 2; 
        unsigned gpsInsManufacturerOUI : 24;
        /// Timestamp in ECEF Ephemeris field and Formatted GPS/INS location field is the same format as the timestamp in 
        /// the VRT packet header, except the `00` value code means the timestamp words are present, but not specified. 
        unsigned integerSecondTimestamp : 32;
        unsigned fractionalSecondTimestamp1 : 32;
        unsigned fractionalSecondTimestamp2 : 32;
        /// Latitude and Longitude words use 32 bit, two's compliment fixed point format with radix point between bits 22 
        /// and 21.
        unsigned latitude : 32;
        unsigned longitude : 32;
        /// Altitude word uses 32 bit, two's compliment fixed point format with radix point between bits 5 and 4. Context
        /// packet class documentation must specify the altitude reference. Typically the height above the WGS-84 
        /// ellipsoid for GPS.
        unsigned altitude : 32;
        /// Speed over ground word uses 32 bit, two's compliment fixed point format with radix point between bits 16 and 
        /// 15, given in m/s. 
        unsigned groundSpeed : 32;
        /// Angle words use 32 bit, two's compliment fixed point format with radix point between bits 22 and 21. 
        unsigned headingAngle : 32;
        unsigned trackAngle : 32;
        /// Magnetic variation word uses 32 bit, two's compliment fixed point format with radix point between bits 22 and
        /// 21. 
        unsigned magneticVariation : 32;
    } formatted_gps_ins_field;
#pragma pack(pop)
    static_assert(sizeof(formatted_gps_ins_field) == 44, "Formatted GPS/INS field is not 416 bits.");

    /// @struct gps_ascii_field
#pragma pack(push, 1)
    typedef struct gps_ascii_field
    {
        unsigned _reserved_bits : 8;
        unsigned gpsManufacturerOUI : 24;
        unsigned numberOfWords : 32;
    };
#pragma pack(pop)

    /// @struct data_packet_format_field
    /// The Data Packet Payload Format field is used to convey parameters that specify the packing and content of the 
    /// payload of the paired Data Packet Stream.
    /// The “Item Packing Field Size,” “Data Item Size,” “Repeat Count,” and “Vector Size” fields contain a number that is
    /// one less than the size or count being used. The “Event-Tag Size” and “Channel-Tag Size” fields contain a number 
    /// equal to the size of those fields
#pragma pack(push, 1)
    typedef struct data_packet_format_field
    {
        /// 31. Data item packing method, determines whether link-efficient format (1) or processing-efficient (0) format 
        /// is used.
        unsigned packMethod : 1;
        /// 30..29. Indicates whether the data samples are real or one of the complex types according to 2 bit code.
        DATA_SAMPLE_TYPE realComplex : 2;
        /// 28..24. Contains 5 bit code to indicate the type of data items used in the paired Data Packet Stream
        DATA_ITEM_FORMAT dataItemFormat : 5;
        /// 23. Set to `1` to indicate when Sample Component Repeating is used in the paired Data Packet Stream.
        unsigned repeatIndicator : 1;
        /// 22..20. Contains an unsigned number equal to the event-tag size used in the Data Packet Stream (big endian).
        unsigned eventTagSize : 3;
        /// 19..16. Contains an unsigned number equal to the channel-tag size used in the Data Packet Stream (big endian).
        unsigned channglTagSize : 4;
        /// 15..12. Applies to the VRT Signed/Unsigned Fixed-Point Non-Normalized numeric formats.
        unsigned dataItemFracSize : 4;
        /// 11..6. Big endian unsigned number that is one less than the actual Item Packing Field Size.
        unsigned itemPackingFieldSize : 6;
        /// 5..0. Big endian unsigned number that is one less than the actual Data Item Size.
        unsigned dataItemSize : 6;
        ///  --- 
        /// **Second Word**
        /// 31..16. Big endian unsigned number that is one less than the actual repeat count in the Data Packet stream. 
        /// Refers to either the Channel Repeating count or the Sample Component Repeating count, only one of these repeat
        /// methods may be used concurrently.
        unsigned repeatCount : 16;
        /// 15..0. Big endian unsigned number that is one less than the actual vector size in the Data Packet stream. 
        unsigned vectorSize : 16;
    } data_packet_format_field;
#pragma pack(pop)
    static_assert(sizeof(data_packet_format_field) == 8, "Data Packet Format Field is not 64 bits.");

    /// @struct cif0
    /// Default included context/command indicator field, bits set to `1` enable fields in context/command packet body.
    /// Legacy fields, CIF enables.
#pragma pack(push, 1)
    typedef struct cif0
    {
        /// 31. Signifies a change in context field values.
        unsigned ctxtFieldChangeIndicator : 1;
        /// 30. Enables 32 bit word containing the stream ID of a reference point.
        unsigned refPointIdentifier : 1;
        /// 29. Describes amount of usable spectum at the output of a process. Enables two 32 bit words with 
        /// radix point between bits 20 and 19 of second word.
        unsigned bandwidth : 1;
        /// 28. Describes centre of band of described signal. Typically used with RF Ref. frequency to describe original 
        /// signal. Enables two 32 bit words with radix point between bits 20 and 19 of second word. 
        unsigned ifRefFreq : 1;
        /// 27. Describes centre of band of original signal. Where `rfRefFreq + rfRefFreqOffset = ifRefFrequency`. 
        /// Enables two 32 bit words with radix point between bits 20 and 19 of second word. See chapter 9.5.10 of 
        /// VITA-49.2-2017.
        unsigned rfRefFreq : 1;
        /// 26. Describes translation frequency to apply to rfRefFreq resulting in IF Ref. freq. See chapter 9.5.10 of 
        /// VITA-49.2-2017. Enables two 32 bit words with radix point between bits 20 and 19 of second word.
        unsigned rfRefFreqOffset : 1;
        /// 25. Specifies frequency offset from IF Reference Frequency to centre of the band. Enables two 32 bit words 
        /// with radix point between bits 20 and 19 of second word.
        unsigned ifBandOffset : 1;
        /// 24. Describes the AC power of a single sine wave at the reference point where the peak amplitude is 1. Enables 
        /// two 32 bit words with radix point between bits 20 and 19 of second word.
        unsigned refLevel : 1;
        /// 23. 
        unsigned gain : 1;
        /// 22. Conveys the number of over-range Data Samples in a single-paired Data Packet. 
        unsigned overRangeCount : 1;
        /// 21. Rate of data samples in the payload of the paired Data Packet stream. Enables two 32 bit words with radix 
        /// point between bits 20 and 19 of second word. 
        unsigned sampleRate : 1;
        /// 20. Used to adjust the timestamp in the signal data packet to take into account both analogue and digital 
        /// processing delays. 
        unsigned timestampAdjustment : 1;
        /// 19. Convays data and time at which thetimestamp in data and context packets was last known to be correct. 
        unsigned timestampCalibrationTime : 1;
        /// 18. Temperature if devuce in Celcius expressed by 16 bit two's complement fixed point value in second half of 
        /// the word. 
        unsigned deviceTemp : 1;
        /// 17. Used to identify the manufacturer and model of the device generating a Control Packet Stream. 
        unsigned deviceID : 1;
        /// 16. Conveys a set of binary indications and a limited number of non-binary state indications.
        unsigned stateEventIndicators : 1;
        /// 15. Used to convey the parameters that specify the packing and content of the payload of the paired Data 
        /// Packet Stream.
        unsigned signalDataPktPayloadFormat : 1;
        /// 14. Array of words describing the position, velocity, and direction of a point (typically a craft) using a 
        /// satellite navigation system.   
        unsigned formattedGPS : 1;
        /// 13. Array of words (matches GPS format) describing the position, velocity, and direction of a point (typically
        /// a craft) using inertial navigation system.
        unsigned formattedINS : 1;
        /// 12. Format to convey location in Earth-Centered, Earth-Fixed cartesian coordinates. Also contains a Cartesian
        /// decomposition of velocity and magnitude. 
        unsigned ecefEphemeris : 1;
        /// 11. Same format as ECEF Ephemeris field, but uses user defined relative ephemeris coordinate. 
        unsigned relativeEphemeris : 1;
        /// 10. Used alongside relative ephemaris to identify the object whose location serves as the ephemeris origin.
        unsigned ephemerisRefID : 1;
        /// 9. Contains GPS scentence information in formatted ASCII string. Should always be accompanied by Formatted GPS
        /// Geolocation field. 
        unsigned gpsAscii : 1;
        /// 8. 
        unsigned ctxtAssociationLists : 1;
        /// 7. Enable CIF7 word (field attributes).
        unsigned cif7Enable : 1;
        /// 6. 5. 4. _reserved bits
        unsigned __reserved1_ : 3;
        /// 3. Enable CIF3 word.
        unsigned cif3Enable : 1;
        /// 2. Enable CIF2 word.
        unsigned cif2Enable : 1;
        /// 1. Enable CIF1 word, to be appended to Context/Command packet header after CIF0.
        unsigned cif1Enable : 1;
        /// 0. _reserved
        unsigned __reserved2_ : 1;
    } cif0;
#pragma pack(pop)
    static_assert(sizeof(cif0) == 4, "CIF0 size is not a single 32 bit word.");

    /// @struct cif1
    /// Context/Command indicator field 1 enabled by CIF0 bit 1, bits set to `1` enable fields in context/command packet 
    /// body.
    /// Spatial, Signal, Spectral, I/O, Ctl.
#pragma pack(push, 1)
    typedef struct cif1
    {
        /// Bit 31. Describes phase difference between reference point and described signal. Enables a 32 bit word 
        /// starting with 16 reserved bits, followed by 16 bit value with radix point between bits 7 and 8. 
        unsigned phaseOffset : 1;
        /// Bit 30. Contains two two's-complement 16-bit values describing tilt and ellipicity angle respectively, with 
        /// radix point between bits 13 and 12. 
        unsigned polarization : 1;
        /// 29. Two 16 bit values signifying Elevation and Azimuth angles.
        unsigned threeDPointingVector : 1;
        /// Bit 28. More complex signifier of 3D pointing vector. Contains three words that describe the structure of the
        /// field. 2 16 bit values with radix point between bits 7 and 8 to describe horizontal and vertical beamwith 
        /// respectively.
        unsigned threeDPointingVectorStruct : 1;
        /// Bit 27. 
        unsigned spacialScanType : 1;
        /// Bit 26. 
        unsigned spacialRefType : 1;
        /// Bit 25. 32 bit field to express -3dB width of the described signal main lobe in the horizontal and vertical
        /// plane. 
        unsigned beamWidth : 1;
        /// Bit 24. 
        unsigned range : 1;
        /// 23..21. _reserved bits.
        unsigned __reserved_bits_ : 3;
        /// Bit 20. Energy-per-bit to Noise density ratio and Bit Error Rate. Enables two 32 bit words with radix point 
        /// between bits 20 and 19 of second word.
        unsigned EbNoBER  : 1;
        /// Bit 19. Provides the ability to set a signal threshold to trigger some signal-level-based action. Action is 
        /// application dependant. Provides for 2 threshold value to be set, supporting a two-level threshold, or a window.
        /// Enables a 32 bit word, formed from 2 16 bit values with radix points between bits 7 and 8. 
        unsigned threshold : 1;
        /// Bit 18. 1dB Compression Point, indicates power limit on device performance. Enables a 32 bit word starting 
        /// with 16 reserved bits, followed by 16 bit value with radix point between bits 7 and 8. 
        unsigned compressionPoint1dB : 1;
        /// Bit 17. Values representing device linearity performance, combined for efficiency. Enables a 32 bit word 
        /// consisting of two 16 bit values with the radix point between the 7th and 8th bits.
        unsigned secondAndThirdOrderInterceptPoint : 1;
        /// Bit 16. Values representing noise performance, combined for efficiency. Enables a 32 bit word consisting of 
        /// two 16 bit values with the radix point between the 7th and 8th bits.
        unsigned snrAndNoiseFigure : 1;
        /// Bit 15. Provides a second frequency field for cases where RF and IF frequency fields are insufficient. Enables
        /// two 32 bit words with radix point between bits 20 and 19 of second word.
        unsigned auxFreq : 1;
        /// Bit 14. Aux Gain provides a second gain field. Enables two 32 bit words with radix point between bits 20 and 
        /// 19 of second word.
        unsigned auxGain : 1;
        /// Bit 13. Provides a second bandwidth field. Enables two 32 bit words with radix point between bits 20 and 19 of 
        /// second word. 
        unsigned auxBandwidth : 1;
        /// Bit 12. _reserved bit.
        unsigned __reserved_2 : 1;
        /// Bit 11. 
        unsigned cifArr : 1;
        /// Bit 10. Multi-word field that describes control or context for spectral information. Uses 13 words. 
        unsigned spectrum : 1;
        /// Bit 9. Provides a way to setup and report a multi-sectored scanning reciever, 
        unsigned sectorScanStep : 1;
        /// Bit 8. _reserved bit.
        unsigned __reserved_3 : 1;
        /// Bit 7. 
        unsigned indxList : 1;
        /// Bit 6. 32 bits of user-defined on/off control. 
        unsigned discreteIO32 : 1;
        /// Bit 5. 64 bits of user-defined on/off control. 
        unsigned discreteIO64 : 1;
        /// Bit 4. Conveys system health attributes. Uses lower 16 bits of a 32 bit word. Any method can be used. 
        unsigned healthStatus : 1;
        /// Bit 3. Identifies particular VITA49 version that is implemented by the packet source. 32 bit word.
        unsigned v49SpecCompliance : 1;
        /// Bit 2. 
        unsigned versionBuildCode : 1;
        /// Bit 1. 
        unsigned bufferSize : 1;
        /// Bit 0. _reserved bit.
        unsigned __reserved_4 : 1;
    } cif1;
#pragma pack(pop)
    static_assert(sizeof(cif1) == 4, "CIF1 size is not a single 32 bit word.");

#pragma pack(push, 1)
    typedef struct bind_field
    {
        unsigned confidence : 8;
        unsigned probabilityFunc : 8;
        unsigned probability : 8;
        unsigned _reserved_bits : 7;
        /// Bit 0. If `1`, indentifiers at the same packet structure level and array index shall be considered bound. If 
        /// `0`, the formerly described indentifiers shall be considered dissacociated. 
        unsigned B : 1;
    } bind_field;
#pragma pack(pop)
    static_assert(sizeof(bind_field) == 4, "Bind field is not a single 32 bit word.");

#pragma pack(push, 1)
    typedef struct country_code_field
    {
        unsigned _reserved_bits1 : 16;
        /// Bit 15. Contains a user defined value. 
        unsigned U : 1;
        unsigned _reserved_bits2 : 3;
        /// Bits 11..0. Contains ISO3166-1 country code, can also hold a user-defined value. 
        unsigned ISO3166_1 : 12;
    } country_code_field;
#pragma pack(pop)
    static_assert(sizeof(country_code_field) == 4, "Country code field is not a single 32 bit word.");

#pragma pack(push, 1)
    typedef struct EMS_device_class
    {
        unsigned generalEMSDeviceType : 16;
        EMS_ORGANISATIONAL_RELATIONSHIP organisationalRelationship : 2; 
        unsigned isExiter : 1;
        unsigned isReciever : 1;
        unsigned EMSType : 12;
    } EMS_device_class;
#pragma pack(pop)
    static_assert(sizeof(EMS_device_class) == 4, "EMS Device Class field is not a single 32 bit word.");

    /// @struct cif2
    /// Context/Command indicator field 2 enabled by CIF0 bit 2, bits set to `1` enable fields in context/command packet 
    /// body.
    /// Identifiers (tags).
#pragma pack(push, 1)
    typedef struct cif2
    {
        /// Bit 31. Used to convey a binding command.
        unsigned bind : 1;
        /// Bit 30. 
        unsigned citedSID : 1;
        /// Bit 29.
        unsigned siblingsSID : 1;
        /// Bit 28.  
        unsigned parentSID : 1;
        /// Bit 27. 
        unsigned childrenSID : 1;
        /// Bit 26. 
        unsigned citedMessageID : 1;
        /// Bit 25. Matches format of controlleeID from command packet header, conveys the Controller ID that last set the
        /// values of the fields in the Context packet. 1 Word. 
        unsigned controlleeID : 1;
        /// Bit 24. Matches format of controlleeUUID from command packet header, conveys the Controller UUID that last set
        /// the values of the fields in the Context packet. 4 Words. 
        unsigned controlleeUUID : 1;
        /// Bit 23. Matches format of controllerID from command packet header, conveys the Controller ID that last 
        /// set the values of the fields in the Context packet. 1 Word. 
        unsigned controllerID : 1; 
        /// Bit 22. Matches format of controllerUUID from command packet header, conveys the Controller UUID that last
        /// set the values of the fields in the Context packet. 4 Words. 
        unsigned controllerUUID : 1;
        /// Bit 21. Asigns a tag to represent different types of information sources. 1 Word. 
        unsigned infoSourceID : 1;
        /// Bit 20. Assigns a common tag to a set of information represented in a context packet. This is a dynamic number
        /// that is generated during operations. Used to associate other VITA49.2 fields that describe an observation to 
        /// a common track number. 1 Word. 
        unsigned trackID : 1;
        /// Bit 19. Contains the ISO 3166-1 Numeric code within the lower 11 bits and a generic 16 bit identifier field. 
        /// Country code standard defined at [http://www.iso.org/iso/home/standards/country_codes.htm]
        unsigned countryCode : 1;
        /// Bit 18. Field shall use the Generic 16 bit Identifier field to provide linkage to operator enumerations in a 
        /// file, database, table or similar. 
        unsigned opID : 1;
        /// Bit 17. Used to link to a generic class of the object. 
        unsigned platformClass : 1;
        /// Bit 16. Used to link to specific objects. 
        unsigned platformInstance : 1;
        /// Bit 15. Used to link to a graphical display item. 
        unsigned platformDisplay : 1;
        /// Bit 14. Used to convey the general type of sensor used to generate the signal such as radar, or a 
        /// communications beacon. The enumeration may also represent sub-classes such a search radar, navigation radar, 
        /// or broadcast. 
        unsigned emsDeviceClass : 1;
        /// Bit 13. Used to identify the specific type of EMS device. 
        unsigned emsDeviceType : 1;
        /// Bit 12. A unique identifier for the specific device emitting the RF signal. 
        unsigned emsDeviceInstance : 1;
        /// Bit 11. Conveys the general type of modulation used via a Generic16 bit Identifier field. 
        unsigned modulationClass : 1;
        /// Bit 10. Conveys specific modulation attributes via a Generic16 bit Identifier field. 
        unsigned modulationType : 1;
        /// Bit 9. Used to enumerate different analog, digital, and other processes performed by SID it is associated with.
        /// Uses the generic 16 bit identifier. 
        unsigned funcID : 1;
        /// Bit 8. Used to enumerate the different modes or states a node associated with the SID it is bound to. Uses the 
        /// generic 16 bit identifier. 
        unsigned modeID : 1;
        /// Bit 7. Used to enumerate different events that impact the interpretation of information or operations. Can 
        /// also be used to represent external information that iompacts observations or operations. Uses the generic 16 
        /// bit identifier. 
        unsigned eventID : 1;
        /// Bit 6. Enables a prioritisation schema to be expressed for using V49.2 Command Packets. Uses the generic 16 
        /// bit identifier. 
        unsigned funcPriorityID : 1;
        /// Bit 5. Sets a priority level to the information to be conveyed a channel. 
        unsigned commPriorityID : 1;
        /// Bit 4. Used to portray the antenna geographical footprint on earth via a Generic 32-Bit Identifier linked to a
        /// KML Class code in a Data Extension Packet.
        unsigned rfFootprint : 1;
        /// Bit 3. Portray the antenna geographical footprint on earth via a Generic 32-Bit Identifier linked to a KML in 
        /// a Data Extension Packet.
        unsigned rfFootprintRange : 1;
        /// 2..0. reserved bits.
        unsigned _reserved_bits : 3;
    } cif2;
#pragma pack(pop)
    static_assert(sizeof(cif2) == 4, "CIF2 size is not a single 32 bit word.");

#pragma pack(push, 1)
    typedef struct sea_and_swell_state_field
    {
        /// Bit 31..16. reserved
        unsigned _reserved_bits : 16;
        unsigned userDefined : 6;
        SEA_STATE_LEVELS swellState : 5;
        SEA_STATE_LEVELS seaState : 5;
    } sea_and_swell_state_field;
#pragma pack(pop)
    static_assert(sizeof(sea_and_swell_state_field) == 4, "Sea/Swell state field is not a single 32 bit word.");

    /// @struct cif3
    /// Context/Command indicator field 3 enabled by CIF0 bit 3, bits set to `1` enable fields in context/command packet 
    /// body.
    /// Temporal/envoronmental. 
#pragma pack(push, 1)
    typedef struct cif3
    {
        /// Bit 31. Enables timestamp details field. 
        unsigned timestampDetails : 1;
        /// Bit 30. Describes a time offset between channels in a system or between systems. 
        unsigned timestampSkew : 1;
        /// Bits 29. 28. reserved.
        unsigned _reserved_bits1 : 2;
        /// Bit 27. Describes the timing characteristics of the rising edge of a waveform.
        unsigned riseTime : 1;
        /// Bit 26. Describes the timing characteristics of the falling edge of a waveform.
        unsigned fallTime : 1;
        /// Bit 25. Descibes the delta time between the Timestamp and an event related to the current stream associated
        /// with  the Stream ID.
        unsigned offsetTime : 1;
        /// Bit 24. Expresses the duration of a pulse
        unsigned pulseWidth : 1;
        /// Bit 23. Exporesses the duration of a cyclical phenomenon.
        unsigned period : 1; 
        /// Bit 22. Expresses the length of time a pattern exists. 
        unsigned duration : 1;
        /// Bit 21. Expresses the length of time a signal exists. 
        unsigned dwell : 1;
        /// Bit 20. Expresses an uncertanty in time measurements, applicable to all timing fields other than timestamp. 
        unsigned jitter : 1;
        /// Bits 19. 18. reserved. 
        unsigned _reserved_bits2 : 2;
        /// Bit 17. Shows how long the information in the context packet has been valid. 
        unsigned age : 1;
        /// Bit 16. Shows how long the information on the current context packet is expected to remain valid. 
        unsigned shelfLife : 1;
        /// Bits 15..8. reserved.
        unsigned __reserved_bits3 : 8;
        /// Bit 7. Temperature in Celcius expressed by 16 bit two's complement fixed point value in second half of the 
        /// word.
        unsigned airTemp : 1;
        /// Bit 6. Temperature in Celcius expressed by 16 bit two's complement fixed point value in each half of the word
        /// respectively.
        unsigned seaGroundTemp : 1;
        /// Bit 5. Percent humidity where 0x0001 is 100/65535 and 0xFFFF is 100%. Uses second half of word, bits 31..16 
        /// reserved. 
        unsigned humidity : 1;
        /// Bit 4. Atmospheric pressure measured in pascals, where 0x0000 is 1/131071 Pascals and 0xFFFF is 131071 Pascals
        /// Bits 31..17 reserved.
        unsigned barPressure : 1;
        /// Bit 3. 
        unsigned seaAndSwellState : 1;
        /// Bit 2. 
        unsigned troposphericState : 1;
        /// Bit 1. Used to associate information on the same network via a generic 32 bit identifier. 
        unsigned networkID : 1;
        /// Bit 0. reserved.
        unsigned _reserved_ : 1;
    } cif3;
#pragma pack(pop)
    static_assert(sizeof(cif3) == 4, "CIF3 size is not a single 32 bit word.");

    /// @struct probability_attribute_subfield
#pragma pack(push, 1)
    typedef struct probability_attribute_subfield
    {
        unsigned _reserved_bits : 16;
        unsigned probabilityFunc : 8;
        unsigned probability : 8;
    } probability_attribute_subfield;
#pragma pack(pop)
    static_assert(sizeof(probability_attribute_subfield) == 4, "Probability Attribute Sub-Field is not a single 32 bit word.");

    /// @struct belief_subfield
#pragma pack(push, 1)
    typedef struct belief_subfield
    {
        unsigned _reserved_bits : 24;
        unsigned belief : 8;
    } belief_subfield;
#pragma pack(pop)
    static_assert(sizeof(belief_subfield) == 4, "Belief Sub-Field is not a single 32 bit word.");

    /// @struct cif7
    /// Context/Command indicator field 7 enabled by CIF0 bit 7, bits set to `1` enable fields in context/command packet 
    /// body.
    /// 
    /// **Attributes.** 
    /// When CIF7 values are selected, additional words matching the length of the original CIF fields shall be inserted
    /// into the Context/Command field after the original CIF value in sequential order as given by CIF7. For example, 
    /// if a packet has CIF0 bits 23 (gain) and 27 (RF Ref. frequency) selected, and CIF7 bits 27 and 26 selected, then
    /// the order of the Context/Command field shall be: Gain -> Max gain -> Min gain -> RF Ref. Freq. -> Max RF Ref. Freq.
    /// -> Min RF Ref. Freq.
#pragma pack(push, 1)
    typedef struct cif7
    {
        /// 31. The value of the field per its original field definitions. 
        unsigned currentValue : 1;
        /// 30. The average value of the field(s).
        unsigned averageValue : 1;
        /// 29. The mean value of the field(s).
        unsigned medianValue : 1;
        /// 28. The standard deviation of the field(s).
        unsigned standardDeviation : 1;
        /// 27. The maximum value of the field(s).
        unsigned maxValue : 1;
        /// 26. The minimum value of the field(s).
        unsigned minValue : 1;
        /// 25. The precision of the field(s) represented as +/- of the field value.
        unsigned precision : 1;
        /// 24. The precision of the field(s) represented as +/- of the field value.
        unsigned accuracy : 1;
        /// 23. 1st Derivative (velocity rate of change). 
        unsigned firstDerivative : 1; 
        /// 22. 2nd Derivative (acceleration). 
        unsigned secondDerivative : 1;
        /// 21. 3rd Derivative. 
        unsigned thirdDerivative : 1;
        /// 20. 
        unsigned probability : 1;
        /// 19. 
        unsigned belief : 1;
        /// 18..0. reserved.
        unsigned _reserved_bits : 19;
    } cif7;
#pragma pack(pop)
    static_assert(sizeof(cif7) == 4, "CIF7 size is not a single 32 bit word.");
/*
    /// @struct control_mode_field
#pragma pack(push, 1)
    typedef struct control_mode_field
    {

    } control_mode_field;
#pragma pack(pop)

    /// @struct acknowledge_mode_field
#pragma pack(push, 1)
    typedef struct acknowledge_mode_field
    {

    } acknowledge_mode_field;
#pragma pack(pop)
*/
    /// @struct cam_ctrl
    /// Control/Acknowledge mode (CAM) field, defines how and when the recipient of a Control packet should react to the 
    /// controls, whether or not an Acknowledge packet or packets should be generated, and how warnings and errors should 
    /// be handled.
    /// Included comments specify setting of CAM field when set to 1, where the opposite or absense of the setting 
    /// described by the field is implied otherwise. If this is not the case, the setting described by the 0 value is 
    /// stated explicitly.
#pragma pack(push, 1)
    typedef struct cam_ctrl
    {
        /// 31. Include controllee identifier
        unsigned controlleeEnable : 1;
        /// 30. Determines format of controllee identifier,
        ///     - When set to `1`: format is 128 bit UUID.
        ///     - When set to `0`: format is 32 bit ID word. 
        unsigned identifierFormatControllee: 1;
        /// 29. Include controller identifier
        unsigned controllerEnable : 1;
        /// 28. Determines format of controller identifier,
        ///     - When set to `1`: format is 128 bit UUID.
        ///     - When set to `0`: format is 32 bit ID word. 
        unsigned identifierFormatController: 1;
        /// 27. Execute any fields that do NOT have any warnings or errors as determined by Ctrl-W and Ctrl-Er (see 
        /// VITA49.2-2017 table 8.3.1.2-1 for more details).
        unsigned P : 1;
        /// 26. Permit execution of fields that generate warnings (see VITA49.2-2017 table 8.3.1.2-1 for more details). 
        unsigned W : 1;
        /// 25. Permit execution of fields that generate errors (see VITA49.2-2017 table 8.3.1.2-1 for more details). 
        unsigned Er : 1;
        /// 24..23 Action bit field
        ACTION_MODES A : 2; 
        /// 22. 
        ///     - When set to `1`: Provide AckV anr/or AckX variants of Acknowledge packet, but only when warnings or 
        ///       errors have occurred as per AckV or AckX bits
        ///     - When set to `0`: Provide AckV anr/or AckX variants of Acknowledge packet in all cases when specified by
        ///       AvkV or AckX bits.
        unsigned nack : 1;
        /// 21. _reserved_
        unsigned _reserved_1: 1;
        /// 20. Request validation acknowledge packet. 
        unsigned reqV : 1;
        /// 19.  Request execution acknowledge packet. 
        unsigned reqX : 1;
        /// 18.  Request query-state acknowledge packet. 
        unsigned reqS : 1;
        /// 17. Request warning fields be sent in execution acknowledge packet varients.
        unsigned reqW : 1;
        /// 16. Request error fields be sent in execution acknowledge packet varients.
        unsigned reqEr : 1;
        /// 15. _reserved_
        unsigned _reserved_2 : 1;
        /// 14..12 Timing control fields, used together to form the Ctrl bit field
        TIMING_CTRL timingCtrl : 3;
        /// 11..0 _reserved_
        unsigned _reserved_3 : 12;
        } cam_ctrl;
#pragma pack(pop)
    static_assert(sizeof(cam_ctrl) == 4, "Control/Acknowledge Mode Field is not a single 32 bit word.");

    /// @struct cam_ack
    /// Control/Acknowledge mode (CAM) field, defines how and when the recipient of a Control packet should react to the 
    /// controls, whether or not an Acknowledge packet or packets should be generated, and how warnings and errors should 
    /// be handled.
    /// Included comments specify setting of CAM field when set to 1, where the opposite or absense of the setting 
    /// described by the field is implied otherwise. If this is not the case, the setting described by the 0 value is 
    /// stated explicitly.
#pragma pack(push, 1)
    typedef struct cam_ack
    {
        /// 31. Include controllee identifier
        unsigned controlleeEnable : 1;
        /// 30. Determines format of controllee identifier,
        ///     - When set to `1`: format is 128 bit UUID.
        ///     - When set to `0`: format is 32 bit ID word. 
        unsigned identifierFormatControllee: 1;
        /// 29. Include controller identifier
        unsigned controllerEnable : 1;
        /// 28. Determines format of controller identifier,
        ///     - When set to `1`: format is 128 bit UUID.
        ///     - When set to `0`: format is 32 bit ID word. 
        unsigned identifierFormatController: 1;
        /// 27. Execute any fields that do NOT have any warnings or errors as determined by Ctrl-W and Ctrl-Er (see 
        /// VITA49.2-2017 table 8.3.1.2-1 for more details).
        unsigned P : 1;
        /// 26. Permit execution of fields that generate warnings (see VITA49.2-2017 table 8.3.1.2-1 for more details). 
        unsigned W : 1;
        /// 25. Permit execution of fields that generate errors (see VITA49.2-2017 table 8.3.1.2-1 for more details). 
        unsigned Er : 1;
        /// 24..23 Action bit field
        ACTION_MODES A : 2; 
        /// 22. 
        ///     - When set to `1`: Provide AckV anr/or AckX variants of Acknowledge packet, but only when warnings or 
        ///       errors have occurred as per AckV or AckX bits
        ///     - When set to `0`: Provide AckV anr/or AckX variants of Acknowledge packet in all cases when specified by
        ///       AvkV or AckX bits.
        unsigned nack : 1;
        /// 21. _reserved_
        unsigned _reserved_1: 1;
        /// 20. Request validation acknowledge packet. 
        unsigned ackV : 1;
        /// 19.  Request execution acknowledge packet. 
        unsigned ackX : 1;
        /// 18.  Request query-state acknowledge packet. 
        unsigned ackS : 1;
        /// 17. Request warning fields be sent in execution acknowledge packet varients.
        unsigned ackW : 1;
        /// 16. Request error fields be sent in execution acknowledge packet varients.
        unsigned ackEr : 1;
        /// 15. _reserved_
        unsigned _reserved_: 1;
        /// 14..12 Timing acknowledgement field, used together to form the Ctrl bit field.
        TIMING_ACK timingAck : 3;
        /// 11. Partial action taken.
        unsigned ackP : 1;
        /// 10. Action executed or scheduled for execution.
        unsigned schX : 1;
        /// 9..8 Reserved for future acknowledge modes, 7..0 reserved for general CAM usage.
        unsigned : 10;
        } cam_ack;
#pragma pack(pop)
    static_assert(sizeof(cam_ack) == 4, "Control/Acknowledge Mode Field is not a single 32 bit word.");

    /// @struct warning_andor_error_field
    /// The AckV and AckX Acknowledge packet may report warnings and errors for each field via the Warning Indicator Field
    /// and/or the Error Indicator Field. 
#pragma pack(push, 1)
    typedef struct warning_andor_error_field {
        /// 31. The field was NOT executed due to a warning or error.
        unsigned notEx : 1;
        /// 30. The field was not executed _properly_ due to a device failure.
        unsigned deviceFail : 1;
        /// 29. The device does not accept this particular control field. 
        unsigned erroneusField : 1;
        /// 28. This supplied field value is beyond the capability or operational range of this device.
        unsigned paramOutOfRange : 1;
        /// 27. The supported field value specifies a level of precision beyond the capability of this device.
        unsigned paramUnsupPrecision : 1;
        /// 26. The field had an invalid setting beyond those specified above.
        unsigned invalidFieldValue : 1;
        /// 25. The controllee was unable to meet the timestamp requirement specified by the [T2, T1, T0] bits for the
        /// specified field.
        unsigned timestampIssue : 1;
        /// 24. The supplied field will cause transmission of hazardous power levels. 
        unsigned hazardousPowerLvl : 1;
        /// 23. The supplied field will cause components to be over driven leading to distortion.
        unsigned distortion : 1;
        /// 22. The suplied field will place the in-band power levels out of compliance.
        unsigned inBandPowerCompliance : 1;
        /// 21. The suplied field will place the out-of-band power levels out of compliance.
        unsigned outOfBandPowerCompliance : 1;
        /// 20. The supplied filed will cause co-site interference between devices in the same location. 
        unsigned coSiteInterferance : 1;
        /// 19. The supplied field will cause interference between devices in the same operational region.
        unsigned regionalInterferance : 1;
        /// 18..13. Reserved bits
        unsigned _reserved_bits : 6;
        /// 12..1. User defined bits 
        unsigned userDefined12 : 1;
        /// 0. Reserved
        unsigned _reserved : 1;
    } warning_andor_error_field;
#pragma pack(pop)
    static_assert(sizeof(warning_andor_error_field) == 4, "Warning and/or error field is not a single 32 bit word.");

    /// @brief Template function to get CIF0 word. 
    template<typename VRT>
    static cif0* getCif0(const VRT* packet);

    /// @brief Template function to set the default CIF (CIF0) for the layer. 
    /// @param[in] cif_word The default CIF (CIF0) to set. 
    /// @return A pointer to the default CIF (CIF0) in the layer. 
    template<typename VRT>
    static cif0* setCif0(const cif0& cif_word, const VRT& packet)
    {
        cif0* cif_0 = reinterpret_cast<cif0*>(packet.m_Data);
        *cif_0 = cif_word;

        return cif_0; 
    }
    
    /// @brief Template function to check if CIF1 Enable bit in CIF0 is set.
    /// @param cif_0 CIF0 Word to check.
    /// @return `true` if CIF1 word is present, `false` otherwise.
    static bool hasCif1(cif0* cif_0)
    {
        return(cif_0->cif1Enable == 0b1 ? true : false); 
    }

    /// @brief Get CIF0 word. 
    template<typename VRT>
    static cif1* getCif1(const VRT& packet);

    /// @brief Template function to set the default CIF (CIF1) for the layer. 
    /// @param[in] cif_word The default CIF (CIF1) to set. 
    /// @return A pointer to the default CIF (CIF1) in the layer. 
    template<typename VRT>
    static cif1* setCif1(const cif1& cif_word, const VRT& packet)
    {
        if(!hasCif1(getCif0()));
            PCPP_LOG_ERROR("[Malformed VITA49.2 packet]: Tried to set CIF1 field while CIF1 enable bit is `0`");
            
        cif1* cif_1 = reinterpret_cast<cif1*>(packet.m_Data);
        *cif_1 = cif_word;

        return cif_1; 
    }

    /// @brief Clear optional Cif1 field. 
    static void clearCif1();

    /// @brief Checks if CIF2 Enable bit in CIF0 is set.
    /// @param cif_0 CIF0 Word to check.
    /// @return `true` if CIF2 word is present, `false` otherwise.
    static bool hasCif2(cif0* cif_0)
    { 
        return(cif_0->cif2Enable == 0b1 ? true : false); 
    }

    /// @brief Get CIF2 word. 
    template<typename VRT>
    static cif2* getCif2(const VRT& packet);

    /// @brief Template function to set the default CIF (CIF2) for the layer. 
    /// @param[in] cif_word The default CIF (CIF2) to set. 
    /// @return A pointer to the default CIF (CIF2) in the layer. 
    template<typename VRT>
    static cif2* setCif2(const cif2& cif_word, const VRT& packet)
    {
        if(!hasCif2(getCif0()));
            PCPP_LOG_ERROR("[Malformed VITA49.2 packet]: Tried to set CIF2 field while CIF2 enable bit is `0`");
            
        cif2* cif_2 = reinterpret_cast<cif2*>(packet.m_Data);
        *cif_2 = cif_word;

        return cif_2; 
    }

    /// @brief Clear optional Cif2 field. 
    static void clearCif2();

    /// @brief Checks if CIF3 Enable bit in CIF0 is set.
    /// @param cif_0 CIF0 Word to check.
    /// @return `true` if CIF3 word is present, `false` otherwise.
    static bool hasCif3(cif3* cif_3)
    { 
        return(cif_0->cif3Enable == 0b1 ? true : false); 
    }

    /// @brief Get CIF3 word. 
    template<typename VRT>
    static cif3* getCif3(const VRT& packet);

    /// @brief Template function to set the default CIF (CIF3) for the layer. 
    /// @param[in] cif_word The default CIF (CIF3) to set. 
    /// @return A pointer to the default CIF (CIF3) in the layer. 
    template<typename VRT>
    static cif3* setCif3(const cif3& cif_word, const VRT& packet)
    {
        if(!hasCif3(getCif0()));
            PCPP_LOG_ERROR("[Malformed VITA49.2 packet]: Tried to set CIF3 field while CIF3 enable bit is `0`");
            
        cif3* cif_3 = reinterpret_cast<cif3*>(packet.m_Data);
        *cif_3 = cif_word;

        return cif_3; 
    }

    /// @brief Clear optional Cif3 field. 
    static void clearCif3();

    /// @brief Checks if CIF7 Enable bit in CIF0 is set.
    /// @param cif_0 CIF0 Word to check.
    /// @return `true` if CIF7 word is present, `false` otherwise.
    static bool hasCif7(cif0* cif_0)
    { 
        return(cif_0->cif7Enable == 0b1 ? true : false); 
    }

    /// @brief Get CIF7 word. 
    template<typename VRT>
    static cif7* getCif7(const VRT& packet);

    /// @brief Template function to set the default CIF (CIF7) for the layer. 
    /// @param[in] cif_word The default CIF (CIF7) to set. 
    /// @return A pointer to the default CIF (CIF7) in the layer. 
    template<typename VRT>
    static cif7* setCif7(const cif7& cif_word, const VRT& packet)
    {
        if(!hasCif7(getCif0()));
            PCPP_LOG_ERROR("[Malformed VITA49.2 packet]: Tried to set CIF7 field while CIF7 enable bit is `0`");
        
        cif7* cif_7 = reinterpret_cast<cif7*>(packet.m_Data);
        *cif_7 = cif_word;

        return cif_7; 
    }

    using namespace Vita49dot2Constants;

    /// @class Vita49dot2Layer
    /// @brief Class representing the VITA49.2 Layer.
    /// Contains VRT header word by default, other packet class words are to be 'packed on top', leaving no padding space
    /// for ommitted optional fields. Should take a child class representing the specific packet type for additional 
    /// prologue fields.
    /// Common mandatory prologue fields vary between packet types, VRT Packet field vs. Packet Type Inclusion Matrix is 
    /// as follows. 
    /// 
    /// |              Field              | Signal Data | Ext. Data | Context | Ext. Context | Control/Ack | Ext. Ctrl/Ack |
    /// |---------------------------------|-------------|-----------|---------|--------------|-------------|---------------|
    /// | Header                          |      M      |     M     |    M    |      M       |      M      |       M       |
    /// | Stream Identifier               |      O      |     O     |    M    |      M       |      M      |       M       |
    /// | Class Identifier                |      O      |     O     |    O    |      O       |      O      |       O       |
    /// | Integer-seconds Timestamp       |      O      |     O     |    O    |      O       |      O      |       O       |
    /// | Fractional-seconds Timestamp    |      O      |     O     |    O    |      O       |      O      |       O       |
    /// | Packet-Specific Prologue Fields |     N/A     |    N/A    |   N/A   |     N/A      |     M/O     |      M/O      |
    /// | Payload                         |      M      |     M     |    M    |      M       |      M      |       M       |
    /// | Trailer                         |      O      |     O     |   N/A   |     N/A      |     M/O     |      M/O      |
    class Vita49dot2Layer : public Layer
    {     
    private:
        bool containsStreamId = false;
        bool containsClassId = false;
        bool containsIntTimestamp = false;
        bool containsFracTimetamp = false;

    public:

        /// A constructor that creates the layer from an existing packet data.
        /// @param[in] data A pointer to the raw data.
        /// @param[in] dataLen Size of the data in bytes.
        /// @param[in] prevLayer A pointer to the previous layer.
        /// @param[in] packet A pointer to the Packet instance where layer will be stored.
        Vita49dot2Layer(uint8_t* data, size_t dataLen, Layer* prevLayer, Packet* packet);

        Vita49dot2Layer(); 

        ~Vita49dot2Layer();

        static bool isDataValid(const uint8_t* data, size_t dataLen);

        /// @brief A method that creates a VITA49.2 layer from packet raw data.
        /// @param[in] data A pointer to the raw data.
        /// @param[in] dataLen Size of the data in bytes.
        /// @param[in] prevLater A pointer to the previous layer.
        /// @param[in] packet A pointer to the Packet instance where layer will be stored.
        /// @return a newly allocated VITA49.2 layer of one of the declared types (according to the message type).
        static Vita49dot2Layer* parseVita49dot2Layer(uint8_t* data, size_t dataLen, Layer* prevLayer, Packet* packet);

        /// @brief Static function that checks whether a port is considered a VRT stream port.
        /// @param[in] port The port number to check.
        /// @return True if `port` is a commonly used VITA49 port number, false otherwise.
        static inline bool isVrtPort(uint16_t port);

        /// @brief A static method that validats the input data.
		/// @param[in] data The pointer to the beginning of a byte stream of an DOIP layer.
		/// @param[in] dataLen The length of the byte stream.
		/// @return True if the data is valid and can represent an DOIP layer.
        static bool isDataValid(uint8_t* data, size_t dataLen);

        /// Should do nothing, VITA should be last layer in packet. 
        void parseNextLayer();

        std::string toString() const;

        /// @brief Gets a pointer to the current packet type for the layer.
        /// @return A pointer to the packet type in this layer.
        V49_PACKET_TYPE getPacketType() const;

        /// @brief Get the type of VITA packet as string (context, command, data).
        /// @return string presentation of the VITA packet type. 
        std::string getPacketTypeAsString() const;

        /// @brief Get the packet prologue length.
        /// @return size of prologue.
        size_t getPrologueLength() const;

        /// @brief Set the length of the packet prologue.
        /// @todo is this neccessary?
        /// @param[in] length the prologue prologue length in 32 bit words.
        void setPrologueLength(size_t length);

        /// @brief Get the packet prologue length.
        /// @return `uint32_t` describing the length of the packet prologue in 32 bit words.
        size_t getPayloadLength() const;

        /// @brief Set the length of the packet payload.
        /// @todo is this neccessary?
        /// @param[in] length the prologue payload length in 32 bit words.
        void setPayloadLength() const;

        /// @brief Gets a pointer to the VRT packet header for this layer. 
        /// @return A pointer to the VRT packet header in the layer. 
        vrthdr* getVrtHeader() const
        {
            return reinterpret_cast<vrthdr*>(m_Data);
        }

        /// @brief Sets the VRT packet header for the layer. 
        /// @param[in] header The VRT packet header to set. 
        /// @return A pointer to the VRT packet header in the layer. 
        vrthdr* setVrtHeader(const vrthdr& header)
        {
            vrthdr* hdr = reinterpret_cast<vrthdr*>(m_Data);
            *hdr = header;

            return hdr; 
        }

        /// @brief Checks whether generic prologue contains StreamID.
        /// @return `true` if StreamID is included according to VRT packet type.
        bool hasStreamId() const
        {
            return !(static_cast<V49_PACKET_TYPE>(getVrtHeader()->packetType) == V49_PACKET_TYPE::DATA || 
                     static_cast<V49_PACKET_TYPE>(getVrtHeader()->packetType) == V49_PACKET_TYPE::EXT_DATA);
        }

        /// @brief Gets Stream ID.
        /// @return Array of bytes representing the Stream ID.
        std::array<uint8_t, VRT_STREAMID_LEN> getStreamId();

        /// @brief Sets Stream ID. 
        /// @param[in] streamId the Stream ID to set. 
        void setStreamId(const std::array<uint8_t, VRT_STREAMID_LEN> streamId);

        /// @brief Clears the StreamID.
        void clearStreamId();

        /// @brief Determine whether generic prologue contains ClassId.
        /// @return `true` if ClassID is included according to VRT header.
        bool hasClassId() const
        {
            return getVrtHeader()->hasClassId == 0b1;
        }

        /// @brief Gets Class ID.
        /// @return Array of bytes representing the Class ID.
        std::array<uint8_t, VRT_CLASSID_LEN> getClassId();

        /// @brief Sets Class ID.
        /// @param[in] classId The Class ID to set.
        void setClassId(const std::array<uint8_t, VRT_CLASSID_LEN> classId);

        /// @brief Clears Class ID.
        void clearClassId();

        /// @brief Determine whether generic prologue contains TimeStamp Integer part.
        /// @return `true` if TimeStamp Integer part is included according to VRT header.
        bool hasIntTimestamp() const
        {
            return static_cast<TSI>(getVrtHeader()->tsi) != TSI::NONE;
        }

        /// @brief Gets the integer part of the packet Timestamp.
        /// @return Array of bytes representing the integer Timestamp.
        std::array<uint8_t, VRT_INT_SECONDS_TIMESTAMP_LEN> getIntTimestamp();

        /// @brief Sets the integer part of the packet Timestamp.
        /// @param[in] intTimestamp Array of bytes representing the integer Timestamp.
        /// @throws std::runtime_error if the integer timestamp is not present.
        void setIntTimestamp(const std::array<uint8_t, VRT_INT_SECONDS_TIMESTAMP_LEN> intTimestamp);

        /// @brief Clears integer part of the packet Timestamp.
        void clearIntTimestamp();

        /// @brief Determine whether generic prologue contains TimeStamp Fractional part.
        /// @return `true` if TimeStamp Fractional part is included according to VRT header.
        bool hasFracTimestamp() const
        {
            return static_cast<TSF>(getVrtHeader()->tsf) != TSF::NONE;
        }

        /// @brief Gets the fractional part of the packet Timesamp.
        /// @return Array of bytes representing the fractional Timestamp.
        std::array<uint8_t, VRT_FRAC_SECONDS_TIMESTAMP_LEN> getFracTimestamp();

        /// @brief Sets the fractional part of the packet Timestamp.
        /// @param[in] fracTimestamp uint64_t representing the fractional timestamp.
        void setFracTimestamp(const std::array<uint8_t, VRT_FRAC_SECONDS_TIMESTAMP_LEN> fracTimestamp);

        /// @brief Clears fractional part of the packet Timestamp.
        void clearFracTimestamp();

    private:

        void setPacketType(V49_PACKET_TYPE packetType);

        static inline bool isValidPacketType(unsigned type);

        static inline bool isPayloadLengthValid(uint32_t payloadLength, size_t dataLen);

#pragma pack(push, 1)
        struct vita49_dot_2_generic_prologue 
        {
            vrthdr packetHeader;

            uint32_t streamId;
            
            uint32_t classId;

            uint32_t intTimeStamp;

            uint64_t fracTimeStamp;
        };
#pragma pack(pop)
        vita49_dot_2_generic_prologue* getGenericPrologue() const
        {
            return reinterpret_cast<vita49_dot_2_generic_prologue*>(m_Data);
        }
        static constexpr size_t MANDATORY_LEN = sizeof(vita49_dot_2_generic_prologue);
    };

    inline bool Vita49dot2Layer::isValidPacketType(unsigned type)
    {
        const V49_PACKET_TYPE packetType = static_cast<V49_PACKET_TYPE>(type);

        switch (packetType)
        {
        case V49_PACKET_TYPE::DATA:
        case V49_PACKET_TYPE::DATA_STREAM_ID:
        case V49_PACKET_TYPE::EXT_DATA:
        case V49_PACKET_TYPE::EXT_DATA_STREAM_ID:
        case V49_PACKET_TYPE::CONTEXT:
        case V49_PACKET_TYPE::EXT_CONTEXT:
        case V49_PACKET_TYPE::COMMAND:
        case V49_PACKET_TYPE::EXT_COMMAND:
            return true;
        
        default:
            PCPP_LOG_DEBUG("[Malformed VITA49.2 packet]: Invalid VITA49.2 packet type: 0x" << std::hex << type);
            return true;
        }
    }

    inline bool Vita49dot2Layer::isVrtPort(uint16_t port)
    {
        auto portAsEnum = static_cast<VITA49_PORTS>(port);
        return (portAsEnum == VITA49_PORTS::TCP_UDP_PORT);
    }

    /// @class Vita49dot2ContextPrologue
    /// @brief Class representing the first part of the VITA49.2 Context Packet, includes VRT header, Context Indicator 
    ///        Field 0 (CIF0), and any Context Fields as described by CIF0 as mandatory included fields. 
    ///
    /// |                         (Bits 31..0, 1 Word)                         |
    /// |----------------------------------------------------------------------| 
    /// |                 Stream Identifier (1 Word, Mandatory)                |
    /// |                 Class Identifier (2 Words, Optional)                 |
    /// |                                   -                                  | 
    /// |             Integer-seconds Timestamp (1 Word, Optional)             | 
    /// |           Fractional-seconds Timestamp (2 Words, Optional)           |
    /// |                                   -                                  |
    /// |          Context-Acknowledge Mode Field (1 Word, Mandatory)          | 
    /// | Additional Context Indicator Fields #s 1-7 (Variable size, optional) |
    /// |                                   -                                  | 
    /// |                 Cotnext Fields (Variable, Mandatory)                 |
    /// |                                   -                                  | 
    /// |----------------------------------------------------------------------|
    /// _Template for Context Packet Prologue_ 
    class Vita49dot2Context : public Vita49dot2Layer
    {
    private: 
        /// @brief Timestamp Mode (TSM) specifier 
        enum timestamp_mode : unsigned int
        {
            FINE = 0b0,
            COARSE = 0b1,
        };

        bool isExt;
        bool hasStreamId;
        bool hasClassId;
        bool hasIntTimestamp;
        bool hasFracTimestamp;

    public: 

        /// @brief Constructs the layer from an existing packet data.
		/// @param[in] data Pointer to raw packet data.
		/// @param[in] dataLen Length of the raw data.
		/// @param[in] prevLayer Pointer to the previous layer.
		/// @param[in] packet Pointer to the parent packet instance.
        Vita49dot2Context(uint8_t* data, size_t dataLen, Layer* prevLayer, Packet* packet);

        // /// @brief Constructs the layer from specified field values. 
        // Vita49dot2Context(uint32_t* streamId, uint64_t* classId, uint32_t* intTimestamp, uint64_t* fraTimestamp, 
        //                   cif0* cif0, cif1* cif1, cif2* cif2, cif3* cif3, cif7* cif7, uint8_t* contextFields)

        static bool isDataValid(const uint8_t* data, size_t dataLen);

        static inline bool isDataLenValid();

        /// @brief Returns true (coarse) or false (fine) depending on VRT header type specific indicator bit 24.
        bool getTSM() const 
        {
            return getVrtHeader()->typeSpecificIndicator2 == 0b1;
        } 

        /// @brief Setter for context packet header bit 24, Timestamp Mode.
        /// @param[in] tsm Enum for timestamp mode, `FINE` or `COARSE` for ease of use.
        void setTSM(timestamp_mode tsm) 
        {
            vrthdr* hdr = getVrtHeader();
            hdr->typeSpecificIndicator1 = tsm;
        }

        /// @return `True` if the packet is NOT compliant with VITA49.0, `False` if the packet is compliant with some 
        /// other VITA49 version. 
        bool isNotAV49dot0() const
        {
            return getVrtHeader()->typeSpecificIndicator2 == 0b1;
        }

        /// @brief Sets whether the packet is NOT compliant with VITA49.0, usually used to confirm interoperability with 
        ///        older systems.
        /// @param[in] isNotV49dot0 If `True`, the packet is not compliant with VITA49.0. Else the packet is compliant 
        /// with some other VITA49 version, usually VITA49.2.  
        void setIsNotAV49dot0(bool isNotV49dot0)
        {
            vrthdr* hdr = getVrtHeader();
            hdr->typeSpecificIndicator2 = isNotAV49dot0? 0b1 : 0b0;
        }

        friend cif0* getCif0(this);

        // /// @brief Get CIF0 word. 
        // cif0* getCif0() const
        // {
        //     return reinterpret_cast<cif0*>(m_Data);
        // }

        /// @brief Sets the default CIF (CIF0) for the layer. 
        /// @param[in] cif_word The default CIF (CIF0) to set. 
        /// @return A pointer to the default CIF (CIF0) in the layer. 
        cif0* setCif0(const cif0& cif_word)
        {
            cif0* cif_0 = reinterpret_cast<cif0*>(m_Data);
            *cif_0 = cif_word;

            return cif_0; 
        }

        /// @brief Checks if CIF1 Enable bit in CIF0 is set.
        /// @param cif_0 CIF0 Word to check.
        /// @return `true` if CIF1 word is present, `false` otherwise.
        bool hasCif1(cif0* cif_0)
        {
            return(cif_0->cif1Enable == 0b1 ? true : false); 
        }

        /// @brief Get CIF0 word. 
        cif1* getCif1() const
        {
            return reinterpret_cast<cif1*>(m_Data);
        }

        /// @brief Sets the default CIF (CIF1) for the layer. 
        /// @param[in] cif_word The default CIF (CIF1) to set. 
        /// @return A pointer to the default CIF (CIF1) in the layer. 
        cif1* setCif1(const cif1& cif_word)
        {
            if(!hasCif1(getCif0()));
                PCPP_LOG_ERROR("[Malformed VITA49.2 packet]: Tried to set CIF1 field while CIF1 enable bit is `0`");
                
            cif1* cif_1 = reinterpret_cast<cif1*>(m_Data);
            *cif_1 = cif_word;

            return cif_1; 
        }

        /// @brief Clear optional Cif1 field. 
        void clearCif1();

        /// @brief Checks if CIF2 Enable bit in CIF0 is set.
        /// @param cif_0 CIF0 Word to check.
        /// @return `true` if CIF1 word is present, `false` otherwise.
        bool hasCif2(cif0* cif_0)
        { 
            return(cif_0->cif2Enable == 0b1 ? true : false); 
        }

        /// @brief Get CIF0 word. 
        cif2* getCif2() const
        {
            return reinterpret_cast<cif2*>(m_Data);
        }

        /// @brief Sets the default CIF (CIF2) for the layer. 
        /// @param[in] cif_word The default CIF (CIF2) to set. 
        /// @return A pointer to the default CIF (CIF2) in the layer. 
        cif2* setCif2(const cif2& cif_word)
        {
            if(!hasCif2(getCif0()));
                PCPP_LOG_ERROR("[Malformed VITA49.2 packet]: Tried to set CIF2 field while CIF2 enable bit is `0`");
               
            cif2* cif_2 = reinterpret_cast<cif2*>(m_Data);
            *cif_2 = cif_word;

            return cif_2; 
        }

        /// @brief Clear optional Cif2 field. 
        void clearCif2();

        /// @brief Checks if CIF3 Enable bit in CIF0 is set.
        /// @param cif_0 CIF0 Word to check.
        /// @return `true` if CIF2 word is present, `false` otherwise.
        bool hasCif3(cif0* cif_0)
        { 
            return(cif_0->cif3Enable == 0b1 ? true : false); 
        }

        /// @brief Get CIF3 word. 
        cif3* getCif3() const
        {
            return reinterpret_cast<cif3*>(m_Data);
        }

        /// @brief Sets the default CIF (CIF3) for the layer. 
        /// @param[in] cif_word The default CIF (CIF3) to set. 
        /// @return A pointer to the default CIF (CIF3) in the layer. 
        cif3* setCif3(const cif3& cif_word)
        {
            if(!hasCif3(getCif0()));
                PCPP_LOG_ERROR("[Malformed VITA49.2 packet]: Tried to set CIF3 field while CIF3 enable bit is `0`");
               
            cif3* cif_3 = reinterpret_cast<cif3*>(m_Data);
            *cif_3 = cif_word;

            return cif_3; 
        }

        /// @brief Clear optional Cif3 field. 
        void clearCif3();

        /// @brief Checks if CIF7 Enable bit in CIF0 is set.
        /// @param cif_0 CIF0 Word to check.
        /// @return `true` if CIF7 word is present, `false` otherwise.
        bool hasCif7(cif0* cif_0)
        { 
            return(cif_0->cif7Enable == 0b1 ? true : false); 
        }

        /// @brief Get CIF3 word. 
        cif7* getCif7() const
        {
            return reinterpret_cast<cif7*>(m_Data);
        }

        /// @brief Sets the default CIF (CIF7) for the layer. 
        /// @param[in] cif_word The default CIF (CIF7) to set. 
        /// @return A pointer to the default CIF (CIF7) in the layer. 
        cif7* setCif7(const cif7& cif_word)
        {
            if(!hasCif7(getCif0()));
                PCPP_LOG_ERROR("[Malformed VITA49.2 packet]: Tried to set CIF7 field while CIF7 enable bit is `0`");
            
            cif7* cif_7 = reinterpret_cast<cif7*>(m_Data);
            *cif_7 = cif_word;

            return cif_7; 
        }

        /// @brief Clear optional Cif7 field. 
        void clearCif7();

        /// @brief Determines whether context fields are included. Returns `true` if any options are set to 1 in cif 
        ///        fields.
        void hasContextFields();

        std::vector<uint32_t> getContextFields();

        /// @brief Sets the context field values. 
        /// @param[in] data vector of uint32_t, as per ANSI/VITA-49.2-2017 specification rule 7.2-2, the vector length 
        ///                 MUST be an integer number of 32-bit words.
        void setContextFields(const std::vector<uint32_t> &data);

        /// @brief Clears the context field values. 
        void clearContextFields();

        /// @brief gets the context field.
        /// @return A vector containing context field bytes. 
        std::vector<uint32_t> getUserDefContext() const;

        // /// Type specific indicator bit 26 reserved
        //
        // bool [RESERVED]() const
        // {
        //     return getVrtHeader()->typeSpecificIndicator2 == 0b1;
        // }

        // void [setRESERVED](bool reserved)
        // {
        //     vrthdr* hdr = getVrtHeader();
        //     hdr->typeSpecificIndicator2 = reserved? 0b1 : 0b0;
        // }
    };

    /// @class Vita49dot2ExtContext
    /// @brief 
    class Vita49dot2ExtContext : public Vita49dot2Layer
    {
    private:

    public:

        /// @brief Constructs the Vita49dot2ExtContext from raw packet data.
        /// @param[in] data Pointer to the raw payload data.
        /// @param[in] dataLen Length of the data buffer.
        /// @param[in] prevLayer Pointer to the previous protocol layer.
        /// @param[in] packet Pointer to the parent packet.
        Vita49dot2ExtContext(uint8_t* data, size_t dataLen, Layer* prevLayer, Packet* packet);

        ~Vita49dot2ExtContext();
    
        /// @brief Sets the user-defined context field values. 
        /// @param[in] data vector of uint32_t, as per ANSI/VITA-49.2-2017 specification rule 7.2-2, the vector length 
        ///                 MUST be an integer number of 32-bit words.
        void setUserDefContext(const std::vector<uint32_t>& data);
        
        /// @brief gets the user-defined context field.
        /// @return A vector containing user-defined context field words. 
        std::vector<uint32_t> getUserDefContext() const;

        /// No `clear` method, handled in class destructor.
    };


    /// @class Vita49dot2CommandPrologue
    /// @brief Class representing the first part of the VITA49.2 Command Packet, includes VRT header, Command Indicator Field 0
    /// (CIF0) and any Command Fields as described by CIF0 as mandatory included fields. 
    /// 
    /// |                         (Bits 31..0, 1 Word)                         |
    /// |----------------------------------------------------------------------|
    /// |                Stream Identifier (1 Word, Mandatory)                 |
    /// |                 Class Identifier (2 Words, Optional)                 |
    /// |                                   -                                  |
    /// |             Integer-seconds Timestamp (1 Word, Optional)             |
    /// |           Fractional-seconds Timestamp (2 Words, Optional)           |
    /// |                                   -                                  | 
    /// |          Control-Acknowledge Mode Field (1 Word, Mandatory)          |
    /// |                    Message ID (1 Word, Mandatory)                    |
    /// |             Controllee ID/UUID (1 or 4 Words, Optional)              |
    /// |                                   -                                  | 
    /// |             Controller ID/UUID (1 or 4 Words, Optional)              |
    /// |                                   -                                  | 
    /// | Command Packet Payload (Content Dependent on Command Packet subtype) |
    /// |----------------------------------------------------------------------|
    class Vita49dot2CommandPrologue : public Vita49dot2Layer
    {
    private: 
        
    public: 

        /// @brief Constructs the layer from an existing packet data.
		/// @param[in] data Pointer to raw packet data.
		/// @param[in] dataLen Length of the raw data.
		/// @param[in] prevLayer Pointer to the previous layer.
		/// @param[in] packet Pointer to the parent packet instance.
        Vita49dot2CommandPrologue(uint8_t* data, size_t dataLen, Layer* prevLayer, Packet* packet);

        static inline bool isDataLenValid(size_t dataLen);

        /// @return `True` if the packet is is an acknowledge packet , `False` otherwise. 
        bool isAcknowledge() const
        {
            return getVrtHeader()->typeSpecificIndicator1 == 0b1;
        }

        /// @brief Sets whether the packet an acknowledge packet (true) or a control packet (false).
        /// @param[in] isAcknowledge If `True`, the packet is an acknowledge packet. 
        void setIsAcknowledge(bool isAcknowledge)
        {
            vrthdr* hdr = getVrtHeader();
            hdr->typeSpecificIndicator1 = isAcknowledge? 0b1 : 0b0;
        }

        // /// Type specific indicator bit 25 reserved
        //
        // bool [RESERVED]() const
        // {
        //     return getVrtHeader()->typeSpecificIndicator2 == 0b1;
        // }

        // void [setRESERVED](bool reserved)
        // {
        //     vrthdr* hdr = getVrtHeader();
        //     hdr->typeSpecificIndicator2 = reserved? 0b1 : 0b0;
        // }

        /// @return `True` if the packet is a cancellation packet, `False` if not. 
        bool getIsCancellationPacket() const
        {
            return getVrtHeader()->typeSpecificIndicator3 == 0b1;
        }

        /// @brief Sets whether the specified data is/is not a cancellation packet.
        /// @param[in] isCancellationPacket Set `True`, to indicate that the packet is a cancellation packet. `False` if 
        /// otherwise.
        void setIsCancellationPacket(bool isCancellationPacket)
        {
            vrthdr* hdr = getVrtHeader();
            hdr->typeSpecificIndicator3 = isCancellationPacket? 0b1 : 0b0;
        }

        /// @brief Gets the Control Acknowledge Mode field of a Control packet.
        /// @throws std::runtime_error if method is used on an Acknowledge packet.
        /// @return cam_ctrl containing Control Acknowledge Mode field.
        cam_ctrl getCAMCtrl();

        /// @brief Sets the Control Acknowledge Mode field of a Control packet.
        /// @param[in] CAM cam_ctrl to set.
        /// @throws std::runtime error if method is used on an Acknowledge packet.
        void setCAMCtrl(cam_ctrl CAM); 

        /// @brief Gets the Control Acknowledge Mode field of an Acknowledge packet.
        /// @throws std::runtime_error if method is used on an Control packet.
        /// @return cam_ack containing Control Acknowledge Mode field.
        cam_ack getCAMAck();

        /// @brief Sets the Control Acknowledge Mode field of an Acknowledge packet.
        /// @param[in] CAM cam_ack to set.
        /// @throws std::runtime error if method is used on an Control packet.
        void setCAMAck(cam_ack CAM);

        /// @brief Set Control Acknowledge Mode field, uses template type param to allow passing of `cam_ctrl` or 
        ///        `cam_ack`. 
        /// @tparam T Template type representing `cam_ctrl` or `cam_ack`. 
        /// @throws std::runtime_error if `T` is not `cam_ctrl` or `cam_ack` type.
        template <typename T> void setControlAcknowledgeMode(T);

        /// @brief Determines whether packet includes 32 bit Controllee ID. 
        /// @return `true` if Controllee ID is included, `false` if not. 
        bool hasControlleeID();

        /// @brief Gets Controllee ID. 
        /// @return uint32_t representing Controllee ID.
        uint32_t getControlleeID();

        /// @brief Sets the packet Controllee ID. 
        /// @param[in] controlleeId Controllee ID to set. 
        void setControlleeID(std::array<uint8_t, VRT_CTRL_ID_LEN> controlleeId);

        /// @brief Clears ControlleeID.
        void clearControlleeID();

        /// @brief Determines whether packet includes 128 bit Controllee UUID.
        /// @return `true` if Controllee UUID is included, `false` if not. 
        bool hasControlleeUUID();

        /// @brief Gets Controllee UUID.
        /// @return Controllee UUID as an array of uint32_t with length 4.
        std::array<uint8_t, VRT_CTRL_UUID_LEN> getControlleeUUID();

        /// @brief Sets the packet Controllee UUID. 
        /// @param[in] controlleeUuid Controllee UUID to set.
        void setControlleeUUID(std::array<uint8_t, VRT_CTRL_UUID_LEN> controlleeUuid);

        /// @brief Clears ControlleeUUID.
        void clearControlleeUUID();

        /// @brief Determines whether packet includes 32 bit Controller ID.
        /// @return `true` if Controller ID is included, `false` if not. 
        bool hasControllerID();

        /// @brief Gets Controller ID. 
        /// @return uint32_t representing Controller ID.
        uint32_t getControllerID();

        /// @brief Sets the packet Controller ID. 
        /// @param[in] controllerId Controller ID to set. 
        void setControllerID(std::array<uint8_t, VRT_CTRL_ID_LEN> controllerId);

        /// @brief Clears ControllerID.
        void clearControllerID();

        /// @brief Determines whether packet includes 128 bit Controller UUID.
        /// @return `true` if Controllee UUID is included, `false` if not. 
        bool hasControllerUUID();

        /// @brief Gets Controller UUID.
        /// @return Controller UUID as an array of uint32_t with length 4.
        std::array<uint8_t, VRT_CTRL_UUID_LEN> getControllerUUID();

        /// @brief Sets the packet Controller UUID. 
        /// @param[in] controllerUuid Controller UUID to set. 
        void setControllerUUID(std::array<uint8_t, VRT_CTRL_UUID_LEN> controllerUuid);

        /// @brief Clears ControllerUUID.
        void clearControllerUUID();

    };

    /// @class Vita49dot2Command
    /// @brief 
    class Vita49dot2CommandControl : public Vita49dot2CommandPrologue
    {

    private:

    public:

        /// @brief Get CIF0 word. 
        cif0* getCif0() const
        {
            return reinterpret_cast<cif0*>(m_Data);
        }

        /// @brief Sets the default CIF (CIF0) for the layer. 
        /// @param[in] cif_word The default CIF (CIF0) to set. 
        /// @return A pointer to the default CIF (CIF0) in the layer. 
        cif0* setCif0(const cif0& cif_word)
        {
            cif0* cif_0 = reinterpret_cast<cif0*>(m_Data);
            *cif_0 = cif_word;

            return cif_0; 
        }

        /// @brief Checks if CIF1 Enable bit in CIF0 is set.
        /// @param cif_0 CIF0 Word to check.
        /// @return `true` if CIF1 word is present, `false` otherwise.
        bool hasCif1(cif0* cif_0)
        {
            return(cif_0->cif1Enable == 0b1 ? true : false); 
        }

        /// @brief Get CIF0 word. 
        cif1* getCif1() const
        {
            return reinterpret_cast<cif1*>(m_Data);
        }

        /// @brief Sets the default CIF (CIF1) for the layer. 
        /// @param[in] cif_word The default CIF (CIF1) to set. 
        /// @return A pointer to the default CIF (CIF1) in the layer. 
        cif1* setCif1(const cif1& cif_word)
        {
            if(!hasCif1(getCif0()));
                PCPP_LOG_ERROR("[Malformed VITA49.2 packet]: Tried to set CIF1 field while CIF1 enable bit is `0`");
                
            cif1* cif_1 = reinterpret_cast<cif1*>(m_Data);
            *cif_1 = cif_word;

            return cif_1; 
        }

        /// @brief Checks if CIF1 Enable bit in CIF0 is set.
        /// @param cif_0 CIF0 Word to check.
        /// @return `true` if CIF1 word is present, `false` otherwise.
        bool hasCif2(cif0* cif_0)
        { 
            return(cif_0->cif2Enable == 0b1 ? true : false); 
        }

        /// @brief Get CIF0 word. 
        cif2* getCif2() const
        {
            return reinterpret_cast<cif2*>(m_Data);
        }

        /// @brief Sets the default CIF (CIF2) for the layer. 
        /// @param[in] cif_word The default CIF (CIF2) to set. 
        /// @return A pointer to the default CIF (CIF2) in the layer. 
        cif2* setCif2(const cif2& cif_word)
        {
            if(!hasCif2(getCif0()));
                PCPP_LOG_ERROR("[Malformed VITA49.2 packet]: Tried to set CIF2 field while CIF2 enable bit is `0`");
               
            cif2* cif_2 = reinterpret_cast<cif2*>(m_Data);
            *cif_2 = cif_word;

            return cif_2; 
        }

        /// @brief Checks if CIF1 Enable bit in CIF0 is set.
        /// @param cif_0 CIF0 Word to check.
        /// @return `true` if CIF1 word is present, `false` otherwise.
        bool hasCif3(cif0* cif_0)
        { 
            return(cif_0->cif3Enable == 0b1 ? true : false); 
        }

        /// @brief Get CIF3 word. 
        cif3* getCif3() const
        {
            return reinterpret_cast<cif3*>(m_Data);
        }

        /// @brief Sets the default CIF (CIF3) for the layer. 
        /// @param[in] cif_word The default CIF (CIF3) to set. 
        /// @return A pointer to the default CIF (CIF3) in the layer. 
        cif3* setCif3(const cif3& cif_word)
        {
            if(!hasCif3(getCif0()));
                PCPP_LOG_ERROR("[Malformed VITA49.2 packet]: Tried to set CIF3 field while CIF3 enable bit is `0`");
               
            cif3* cif_3 = reinterpret_cast<cif3*>(m_Data);
            *cif_3 = cif_word;

            return cif_3; 
        }

        /// @brief Checks if CIF1 Enable bit in CIF0 is set.
        /// @param cif_0 CIF0 Word to check.
        /// @return `true` if CIF1 word is present, `false` otherwise.
        bool hasCif7(cif0* cif_0)
        { 
            return(cif_0->cif7Enable == 0b1 ? true : false); 
        }

        /// @brief Get CIF3 word. 
        cif7* getCif7() const
        {
            return reinterpret_cast<cif7*>(m_Data);
        }

        /// @brief Sets the default CIF (CIF7) for the layer. 
        /// @param[in] cif_word The default CIF (CIF7) to set. 
        /// @return A pointer to the default CIF (CIF7) in the layer. 
        cif7* setCif7(const cif7& cif_word)
        {
            if(!hasCif7(getCif0()));
                PCPP_LOG_ERROR("[Malformed VITA49.2 packet]: Tried to set CIF7 field while CIF7 enable bit is `0`");
            
            cif7* cif_7 = reinterpret_cast<cif7*>(m_Data);
            *cif_7 = cif_word;

            return cif_7; 
        }

    };

    /// @class Vita49dot2ExtCommand
    /// @brief 
    class Vita49dot2ExtCommand : public Vita49dot2Layer
    {
    private:
    
    public:

        /// @brief Get CIF0 word. 
        cif0* getCif0() const
        {
            return reinterpret_cast<cif0*>(m_Data);
        }

        /// @brief Sets the default CIF (CIF0) for the layer. 
        /// @param[in] cif_word The default CIF (CIF0) to set. 
        /// @return A pointer to the default CIF (CIF0) in the layer. 
        cif0* setCif0(const cif0& cif_word)
        {
            cif0* cif_0 = reinterpret_cast<cif0*>(m_Data);
            *cif_0 = cif_word;

            return cif_0; 
        }
        
        /// @brief Checks if CIF1 Enable bit in CIF0 is set.
        /// @param cif_0 CIF0 Word to check.
        /// @return `true` if CIF1 word is present, `false` otherwise.
        bool hasCif1(cif0* cif_0)
        {
            return(cif_0->cif1Enable == 0b1 ? true : false); 
        }

        /// @brief Get CIF0 word. 
        cif1* getCif1() const
        {
            return reinterpret_cast<cif1*>(m_Data);
        }

        /// @brief Sets the default CIF (CIF1) for the layer. 
        /// @param[in] cif_word The default CIF (CIF1) to set. 
        /// @return A pointer to the default CIF (CIF1) in the layer. 
        cif1* setCif1(const cif1& cif_word)
        {
            if(!hasCif1());
                PCPP_LOG_ERROR("[Malformed VITA49.2 packet]: Tried to set CIF1 field while CIF1 enable bit is `0`");
                
            cif1* cif_1 = reinterpret_cast<cif1*>(m_Data);
            *cif_1 = cif_word;

            return cif_1; 
        }

        /// @brief Checks if CIF1 Enable bit in CIF0 is set.
        /// @param cif_0 CIF0 Word to check.
        /// @return `true` if CIF1 word is present, `false` otherwise.
        bool hasCif2(cif0* cif_0)
        { 
            return(cif_0->cif2Enable == 0b1 ? true : false); 
        }

        /// @brief Get CIF0 word. 
        cif2* getCif2() const
        {
            return reinterpret_cast<cif2*>(m_Data);
        }

        /// @brief Sets the default CIF (CIF2) for the layer. 
        /// @param[in] cif_word The default CIF (CIF2) to set. 
        /// @return A pointer to the default CIF (CIF2) in the layer. 
        cif2* setCif2(const cif2& cif_word)
        {
            if(!hasCif2());
                PCPP_LOG_ERROR("[Malformed VITA49.2 packet]: Tried to set CIF2 field while CIF2 enable bit is `0`");
               
            cif2* cif_2 = reinterpret_cast<cif2*>(m_Data);
            *cif_2 = cif_word;

            return cif_2; 
        }

        /// @brief Checks if CIF1 Enable bit in CIF0 is set.
        /// @param cif_0 CIF0 Word to check.
        /// @return `true` if CIF1 word is present, `false` otherwise.
        bool hasCif3(cif0* cif_0)
        { 
            return(cif_0->cif3Enable == 0b1 ? true : false); 
        }

        /// @brief Get CIF3 word. 
        cif3* getCif3() const
        {
            return reinterpret_cast<cif3*>(m_Data);
        }

        /// @brief Sets the default CIF (CIF3) for the layer. 
        /// @param[in] cif_word The default CIF (CIF3) to set. 
        /// @return A pointer to the default CIF (CIF3) in the layer. 
        cif3* setCif3(const cif3& cif_word)
        {
            if(!hasCif3());
                PCPP_LOG_ERROR("[Malformed VITA49.2 packet]: Tried to set CIF3 field while CIF3 enable bit is `0`");
               
            cif3* cif_3 = reinterpret_cast<cif3*>(m_Data);
            *cif_3 = cif_word;

            return cif_3; 
        }

        /// @brief Checks if CIF1 Enable bit in CIF0 is set.
        /// @param cif_0 CIF0 Word to check.
        /// @return `true` if CIF1 word is present, `false` otherwise.
        bool hasCif7(cif0* cif_0)
        { 
            return(cif_0->cif7Enable == 0b1 ? true : false); 
        }

        /// @brief Get CIF3 word. 
        cif7* getCif7() const
        {
            return reinterpret_cast<cif7*>(m_Data);
        }

        /// @brief Sets the default CIF (CIF7) for the layer. 
        /// @param[in] cif_word The default CIF (CIF7) to set. 
        /// @return A pointer to the default CIF (CIF7) in the layer. 
        cif7* setCif7(const cif7& cif_word)
        {
            if(!hasCif7());
                PCPP_LOG_ERROR("[Malformed VITA49.2 packet]: Tried to set CIF7 field while CIF7 enable bit is `0`");
            
            cif7* cif_7 = reinterpret_cast<cif7*>(m_Data);
            *cif_7 = cif_word;

            return cif_7; 
        }

    };

    /// @class Vita49dot2Data
    /// @brief Class representing a VITA49.2 Data Packet, includes heder and prologue up to the data payload.
    ///
    /// | (Bits 31..0, 1 Word) |
    /// | - | 
    /// | Stream Identifier (1 Word, Optional) |
    /// | Class Identifier (2 Words, Optional) | 
    /// | Class Identifier Second Word | 
    /// | Integer-seconds Timestamp (1 Word, Optional) | 
    /// | Fractional-seconds Timestamp (2 Words, Optional) |
    /// | Fractional-seconds Timestamp Second word | 
    /// | _# Data Payload_ | 
    /// | - | 
    /// _Template for Signal Data Packet_ 
    class Vita49dot2Data : public Vita49dot2Layer
    {
    private: 
        
    public: 
        
        /// @return `True` if the packet is specified to include trailer, `False` otherwise. 
        bool hasTrailer() const
        {
            return getVrtHeader()->typeSpecificIndicator1 == 0b1;
        }

        /// @brief Sets whether the packet is specified to include a trailer.
        /// @param[in] hasTrailer If `True`, the packet is specified to include a 32 bit trailer word after packet payload. 
        void setHasTrailer(bool hasTrailer)
        {
            vrthdr* hdr = getVrtHeader();
            hdr->typeSpecificIndicator1 = hasTrailer? 0b1 : 0b0;
        }

        /// @return `True` if the packet is NOT compliant with VITA49.0, `False` if the packet is compliant with some 
        /// other VITA49 version. 
        bool isNotAV49dot0() const
        {
            return getVrtHeader()->typeSpecificIndicator2 == 0b1;
        }

        /// @brief Sets whether the packet is NOT compliant with VITA49.0.
        /// @param[in] isNotV49dot0 If `True`, the packet is not compliant with VITA49.0. Else the packet is compliant 
        /// with some other VITA49 version.  
        void setIsNotAV49dot0(bool isNotV49dot0)
        {
            vrthdr* hdr = getVrtHeader();
            hdr->typeSpecificIndicator2 = isNotAV49dot0? 0b1 : 0b0;
        }

        /// @return `True` if the packet contains spectrum data, `False` if the packet contains time data. 
        bool getIsSpectrumData() const
        {
            return getVrtHeader()->typeSpecificIndicator3 == 0b1;
        }

        /// @brief Sets whether the specified data packet contains spectrum or time data.
        /// @param[in] isSpectrumData If `True`, the packet contains spectrum data. Else the packet contains time data. 
        void setIsSpectrumData(bool isSpectrumData)
        {
            vrthdr* hdr = getVrtHeader();
            hdr->typeSpecificIndicator3 = isSpectrumData? 0b1 : 0b0;
        }

        static inline bool isDataLenValid(size_t dataLen);
    };

} /// END NAMESPACE pcpp
