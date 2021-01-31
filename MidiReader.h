#ifndef MIDI_READER_h
#define MIDI_READER_h


#include <vector>
#include <stdint.h>
#include <fstream>
#include <string>
#include <iostream>
#include <memory>


//const int BUFF_SIZE = 1024;

enum Format : int16_t
{
    MIDI_SINGLE = 0,
    MIDI_MULTIPLE = 1,
    MIDI_PATTERN = 2
};

enum Type : char
{
    META_SEQUENCE_NUM = 0,
    META_TEXT = 1,
    META_COPYRIGHT = 2,
    META_SEQUENCE_NAME = 3,
    META_INSTRUMENT_NAME = 4,
    META_LYRIC = 5,
    META_MARKER = 6,
    META_CUE_POINT = 7,
    META_PROGRAM_NAME = 8,
    META_DEVICE_NAME = 9,
    META_MIDI_CHANNEL_PREFIX = 0x20,
    META_MIDI_PORT = 0x21,
    META_END_OF_TRACK = 0x2f,
    META_TEMPO = 0x51,
    META_SMPTE_OFFSET = 0x54,
    META_TIME_SIGNATURE = 0x58,
    META_KEY_SIGNATURE = 0x59,
    META_SEQUENCER_EVENT = 0x7f
};

struct MidiHeader
{
    char m_magic[4];
    int32_t m_seclen;
    Format m_format;
    int16_t m_ntracks;
    int16_t m_tickdiv;


    // 防止内存对齐导致读取错误
    static int get_header_size(){ return sizeof(char) * 4 + sizeof(uint32_t) + sizeof(Format) + sizeof(short) + sizeof(short); }
    MidiHeader(){};
};

//#define TEST
#ifndef TEST
struct DeltaTime
{
    uint32_t total = 0;
    char t0;
    char t1;
    char t2;
    char t3;

    DeltaTime(){};
    friend std::ostream &operator << (std::ostream &os, const DeltaTime &dt)
    {
        os << "t0 =" << dt.t0;
        return os;
    }
};

//80 松开音符
//90 按下音符
//A0 触后音符
//B0 控制器
//C0 改变乐器
//D0 触后通道
//E0 滑音
//F0 系统码
//FF 01 文字
//FF 02 版权
//FF 03 序列或名称
//FF 04 乐器
//FF 05 歌词
//FF 06 标记
//FF 07 提示点
//FF 20 MIDI通道
//FF 21 MIDI端口
//FF 2F MTrk结束标记
//FF 51 速度
//FF 54 SMPTE偏移     FF 54 05 hr mn se fr ff
//FF 58 时间签名      FF 58 04 nn dd cc bb
//FF 59 密钥签名      FF 59 02 sf mi
//FF 7F 专有/其他数据


struct MetaEvent
{
    Type m_type;
    DeltaTime m_length;
    
    //可选
    short m_seqNum;
    std::string m_text;             //FF 01文字
    std::string m_copyright;        //FF 02版权
    std::string m_name;             //FF 03序列或名称
    std::string m_instrument;       //FF 04乐器
    std::string m_lyric;            //FF 05歌词
    std::string m_marker;           //FF 06标记
    std::string m_cuePoint;         //FF 07提示点
    std::string m_programName;      //
    std::string m_deviceName;       //
    char m_channelPrefix;
    char m_port;
    uint32_t m_usecPerQuarterNote : 24; //位域24
    uint32_t m_bpm = 60000000 / m_usecPerQuarterNote;
    char m_hours;
    char m_mins;
    char m_secs;
    char m_fps;
    char m_fracFrames;
    char m_numerator;
    char m_denominator;
    char m_clocksPerClick;
    char m_32ndPer4th;
    char m_flatsSharps;
    char m_majorMinor;
    std::string m_data;

    MetaEvent(){};
};

// message inner structs
struct NoteOffEvent
{
    char m_note;
    char m_velocity;

    NoteOffEvent(){};
};

struct NoteOnEvent
{
    char m_note;
    char m_velocity;

    NoteOnEvent(){};
};

struct NotePressureEvent
{
    char m_note;
    char m_pressure;

    NotePressureEvent(){};
};

struct ControllerEvent
{
    char m_controller;
    char m_value;

    ControllerEvent(){};
};

struct ProgramEvent
{
    char m_program;

    ProgramEvent(){};
};

struct ChannelPressureEvent
{
    char m_pressure;

    ChannelPressureEvent(){};
};

struct PitchBendEvent
{
    char m_lsb;
    char m_msb;

    PitchBendEvent(){};
};

struct SysexEvent
{
    DeltaTime m_length;
    std::string m_message;

    SysexEvent(){};
};

struct MidiMessage
{
    DeltaTime m_dtime;
    char m_status;
    char lastStatus = 0;

    // 可选
    char m_channel;
    NoteOffEvent note_off_event;
    NoteOnEvent note_on_event;
    NotePressureEvent note_pressure_event;
    ControllerEvent controller_event;
    ProgramEvent program_event;
    ChannelPressureEvent channel_pressure_event;
    PitchBendEvent pitch_bend_event;
    MetaEvent meta_event;
    SysexEvent sysex_event;

    MidiMessage(){};
};

struct MidiTrack
{
    char m_magic[4];
    uint32_t m_seclen;
    std::vector<MidiMessage> m_midi_messages;

    MidiTrack(){};
};

struct MidiFile
{
    MidiHeader header;
    MidiTrack tracks;// [header.m_ntracks];

    MidiFile(){};
};
#endif



class MidiReader_file_interface
{
private:
    int size;// 文件总大小 file size
    int pos;// 当前读到的位置 pos
    std::ifstream F;//文件对象 file
    std::shared_ptr<char> buf;//缓冲区 buf

public:
    MidiReader_file_interface()
    {
        size = 0;
        pos = 0;
    }
    MidiReader_file_interface(std::string path)
    {
        size = 0;
        pos = 0;
        read(path);
    }
    ~MidiReader_file_interface()
    {
        if (F.is_open())
        {
            F.close();
        }
    }

    //文件的打开与关闭
    bool read(std::string file_path);

    //读取数据
    bool _get_byte(char* data_buf,int byte_num);//从文件中读取指定字节大小的内容
    bool get(void* addr, size_t len, bool is_char);
    bool get_str(std::string& str, size_t len);

    int get_pos()//当前读取所在位置
    {
        return pos;
    }
    bool eof()
    {
        if (pos == size)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
};



class MidiReader
{
public:
    /*
    bool open_file(std::string file_path);
    bool read_file(MidiFile &file);
    */
    // len指定读取的结构大小 is_char如果为true则不进行大小端转换
    /*
    template<typename T> bool read_var(const T &t, void* addr, size_t len = 0, bool is_char = false);
    bool read_str(std::string &str, size_t len);
    */
    bool read_header();
    bool read_tracks();

    //屏幕输出 print
    void print_header(const MidiHeader &header);
    void print_tracks(const MidiTrack &tracks);
    void print_file(const MidiFile &file);

    MidiReader();
    MidiReader(std::string file_path);
    ~MidiReader();

    MidiFile midi;

    // private:
public:
    
    MidiReader_file_interface MF_interface;
    int file_size;// 文件总大小 file size
    
    int current_pos;// 当前读到的位置 location

    char test_s[5];
    bool read_ok;

    std::ifstream fs;//文件对象 file

    std::shared_ptr<char> buff;//缓冲区 buf

    bool is_file_opened(){ return fs.is_open(); }
    bool read(int byte_num);

    bool read(std::string path);

    void buff_clean();//清空缓冲区 clean data buffer
    void init();
    // track inner
    bool read_messages(MidiMessage &message);
    bool read_delta_time(DeltaTime &dt);
    bool read_meta_event(MetaEvent &me);
    bool read_sysex_event(SysexEvent &se);

};


#endif