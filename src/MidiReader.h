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
//B0 控制器变化
//C0 改变乐器
//D0 通道触动压力
//E0 滑音
//F0 系统码
//FF 00 设置轨道音序
//FF 01 文本信息
//FF 02 版权信息
//FF 03 音轨名称或歌曲名称
//FF 04 乐器
//FF 05 歌词
//FF 06 标记
//FF 07 提示点/注释
//FF 20 MIDI通道
//FF 21 MIDI端口
//FF 2F MTrk结束标记
//FF 51 速度
//FF 54 SMPTE偏移     FF 54 05 hr mn se fr ff
//FF 58 时间签名      FF 58 04 nn dd cc bb
//FF 59 密钥签名      FF 59 02 sf mi
//FF 7F 专有/其他数据
/*
enum env_type : char
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
*/
namespace midi_reader
{
    enum midi_message_type :unsigned char
    {
        release = 0x80, //松开音符
        push = 0x90, //按下音符
        touch = 0xA0, //触后音符
        controller = 0xB0, //控制器变化
        instrument = 0xC0, //改变乐器
        pressure = 0xD0, //通道触动压力
        slip = 0xE0, //滑音
        system = 0xF0  //系统数据
        //FF 00 设置轨道音序
        //FF 01 文本信息
        //FF 02 版权信息
        //FF 03 音轨名称或歌曲名称
        //FF 04 乐器
        //FF 05 歌词
        //FF 06 标记
        //FF 07 提示点/注释
        //FF 20 MIDI通道
        //FF 21 MIDI端口
        //FF 2F MTrk结束标记
        //FF 51 速度
        //FF 54 SMPTE偏移     FF 54 05 hr mn se fr ff
        //FF 58 时间签名      FF 58 04 nn dd cc bb
        //FF 59 密钥签名      FF 59 02 sf mi
        //FF 7F 专有/其他数据
    };
}
enum notes
{
    C=0,
    CX,
    D,
    DX,
    E,
    F,
    FX,
    G,
    GX,
    A,
    AX,
    B,
};


class midi_notes
{
public:
    int press;                      //压力
    notes note;                     //音符
    int octave;                     //八度 , (-1到9)
    midi_notes()
    {
    
    }
    midi_notes(unsigned char _note_octave,unsigned char _press)
    {
        press = _press;
        note = (notes)(_note_octave % 12);
        octave = (_note_octave / 12) - 1;
    }

    std::string notes_str(void)
    {
        std::string str;
        switch (note)
        {
        case 0:
            str = "C";
            break;
        case 1:
            str = "C#";
            break;
        case 2:
            str = "D";
            break;
        case 3:
            str = "D#";
            break;
        case 4:
            str = "E";
            break;
        case 5:
            str = "F";
            break;
        case 6:
            str = "F#";
            break;
        case 7:
            str = "G";
            break;
        case 8:
            str = "G#";
            break;
        case 9:
            str = "A";
            break;
        case 10:
            str = "A#";
            break;
        case 11:
            str = "B";
            break;
        }
        return str;
    }
    void print()
    {
        printf(( "音符：" + notes_str()+" \t").c_str());
        printf("八度：%d \t", octave);
        printf("力度：%d \t", press);
    }
};

class midi_event
{
public:
    uint32_t delay ;
    unsigned char message ;
    unsigned char message_ex ;
    uint32_t data_size ;
    std::shared_ptr <unsigned char> message_data;
    midi_event()
    {
        delay = 0;
        message = 0;
        message_ex = 0;
        data_size = 0;
    };
    char* env_read(char *s)
    {
        {
            int i = 0;
            delay = 0;
            do
            {
                delay <<= 7;
                i = *s;
                delay += i & 0x7F;
                s++;
            } while (i & 0x80);
        }

        message = *s;
        s++;
        if (message == 0xff)
        {
            message_ex = *s;
            s++;
            data_size = *s;
            s++;
        }
        else
        {
            int i = _message_length();
            if (i != 0)
            {
                data_size = i;
            }
            else
            {
                int i=0;
                message_ex = *s;
                s++;
                data_size = 0;
                do
                {
                    data_size <<= 7;
                    i = *s;
                    data_size += i&0x7F;
                    s++;
                } while (i & 0x80);
            }
        }
        message_data.reset(new unsigned char[data_size]);
        memcpy_s(message_data.get(), data_size, s, data_size);
        s += data_size;
        return s;
    };
    ~midi_event()
    {
    };

    int _message_length()
    {
        int length = 0;
        switch (message & (0xf0))
        {
        case midi_reader::release:
            length = 2;
            break;
        case midi_reader::push:
            length = 2;
            break;
        case midi_reader::touch:
            length = 2;
            break;
        case midi_reader::controller:
            length = 2;
            break;
        case midi_reader::instrument:
            length = 1;
            break;
        case midi_reader::pressure:
            length = 1;
            break;
        case midi_reader::slip:
            length = 2;
            break;
        case midi_reader::system:
            length = 0;
            break;
        }
        return length;
    }

    midi_reader::midi_message_type data_type()
    {
        return (midi_reader::midi_message_type)(message & 0xf0);
    }

    void print_env()
    {
        printf("延时:%8d\t", delay);
        printf("音轨:%d\t", message&0x0f);
        unsigned char* d = message_data.get();
        switch (message & (0xf0))
        {
        case midi_reader::release:
        {
            midi_notes note(d[0], d[1]);
            printf("-松开按键\t");
            note.print();
            printf("\n");
        }
            break;
        case midi_reader::push:
        {
            midi_notes note(d[0], d[1]);
            printf("+按下按键\t");
            note.print();
            printf("\n");
        }
            break;
        case midi_reader::touch:
        {
            midi_notes note(d[0], d[1]);
            printf("触后音符\t");
            note.print();
            printf("\n");
        }
            break;
        case midi_reader::controller:
        {
            printf("改变控制器\t");
            printf("控制器号码：%d\t", d[0]);
            printf("控制器参数：0x%2X\t", d[1]);
            printf("\n");
        }
            break;
        case midi_reader::instrument:
        {
            printf("改变乐器\t");
            printf("乐器号码：%d\t",d[0]);
            printf("\n");
        }
            break;
        case midi_reader::pressure:
        {
            printf("通道触动压力\t");
            printf("参数：0x%2X\t", d[0]);
            printf("\n");
        }
            break;
        case midi_reader::slip:
        {
            printf("滑音\t");
            printf("\n");
        }
            break;
        case midi_reader::system:
        {
            printf("系统信息\t");
            printf("\n");
        }
            break;
        }
    }

    void print()
    {
        print_env();
        /*
        if (message == 0xff)
        {
            printf("delay:%d \tmessage:%02X %02X \tsize:%02X ", delay, message, message_ex, data_size);
            
        }
        else
        {

            printf("delay:%d \tmessage:%02X \tsize:%02X ", delay, message, data_size);
        }
        if (data_size != 0)
        {
            printf("\tdata: ");
            unsigned int i = 0;
            for (i = 0; i < data_size; i++)
            {
                printf("%02X ", message_data.get()[i]);
            }
            
        }
        else
        {
            printf("\tno data");
        }
        std::cout << std::endl;
        */
    }

};
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




class MidiMessage
{
public:
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

class MidiTrack
{
public:
    char m_magic[4];
    uint32_t m_seclen;
    //std::vector<MidiMessage> m_midi_messages;
    std::vector<midi_event> events;
    MidiTrack(){};

    void print_env()
    {
        std::vector <midi_event> ::iterator E;
        std::cout << "--->events:" << events.size() << std::endl;
        for (E = events.begin(); E != events.end(); E++)
        {
            E->print();
        }

    }
};

struct MidiFile
{
    MidiHeader header;
    std::vector<MidiTrack> tracks;// [header.m_ntracks];

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
    bool _get_byte(char* data_buf,int byte_num, bool move);//从文件中读取指定字节大小的内容
    bool get(void* addr, size_t len, bool is_char, bool move);
    bool get_str(std::string& str, size_t len);


    int get_pos()//当前读取所在位置
    {
        return pos;
    }
    void set_pos(int _pos)//当前读取所在位置
    {
        pos = _pos;
    }
    void move_pos(int move)//当前读取所在位置
    {
        int _pos = pos + move;
        if((_pos >=0)&&(_pos <=size))
            set_pos(_pos);
    }
    char* get_buf()
    {
        return buf.get()+pos;
    }
};



class MidiReader
{
private:

    bool read_header();
    bool read_tracks();

    //屏幕输出 print
    void print_header();
    void print_tracks();
    


    

    MidiFile midi;
    MidiReader_file_interface MF_interface;
    // private:
public:
    MidiReader();
    MidiReader(std::string file_path);
    ~MidiReader();

    bool read(std::string path);
    void print();
    /*
    // track inner
    bool read_messages(MidiMessage &message);
    bool read_delta_time(DeltaTime &dt);
    bool read_meta_event(MetaEvent &me);
    bool read_sysex_event(SysexEvent &se);*/
    
};


#endif