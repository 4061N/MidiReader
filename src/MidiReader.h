#ifndef MIDI_READER_h
#define MIDI_READER_h


#include <vector>
#include <stdint.h>
#include <fstream>
#include <string>
#include <iostream>
#include <memory>


class MidiReader;

namespace _midi_reader
{
    class midi_note;
    class midi_event_notes;
    class midi_event;
    class midi_track;
    class midi_file_interface;
    class midi_file;
    class midi_wav_maker;
    
#define mid_A_fq 440            //标准A的频率
#define mid_A_data 57           //标准A的位置
//#define mid_A_data 45         //标准A的位置
#define WAV_Attenuation 0.98           //WAV文件中的最大衰减系数
#define WAV_Attenuation_no_touch 0.97   //WAV文件中的延音衰减系数
        



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

        MidiHeader() {};
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
 




    class midi_note
    {
    public:
        long double begin;              //开始时间 ms
        long double end;                //结束时间 ms
        double time;                    //持续时间 ms
        double frequency;               //频率HZ
        unsigned char press;            //力度（0到127）
        unsigned char note_octave;
        midi_note()
        {
        }
        midi_note(long double _begin, long double _end, unsigned char _note_octave, unsigned char _press)
        {
            set(_begin, _end, _note_octave, _press);
        }
        void set_frequency(unsigned char _note_octave)
        {
            frequency = mid_A_fq * pow(2, ((double)_note_octave - mid_A_data) / 12);
        }
        void set(long double _begin, long double _end, unsigned char _note_octave, unsigned char _press)
        {
            begin = _begin;
            end = _end;
            time = end - begin;
            note_octave = _note_octave;
            set_frequency(_note_octave);
            press = _press;
        }
        void print();

        bool operator <(const midi_note& in)
        {
            if (begin < in.begin)
                return true;
            else
                return false;
        }
        bool operator >(const midi_note& in)
        {
            if (begin > in.begin)
                return true;
            else
                return false;
        }
    };



    class midi_event_notes
    {
    public:
        int press;                      //压力
        int note;                     //音符
        int octave;                     //八度 , (-1到9)

        const std::string note_name[12] = { "C" ,"C#" ,"D" ,"D#" ,"E" ,"F" ,"F#" ,"G" ,"G#" ,"A" ,"A#" ,"B" };
        midi_event_notes()
        {

        }
        midi_event_notes(unsigned char _note_octave, unsigned char _press)
        {
            press = _press;
            note = (_note_octave % 12);
            octave = (_note_octave / 12) - 1;
        }

        std::string notes_str(void)
        {
            std::string str = note_name[note];
            return str;
        }
        void print();
    };

    class midi_event
    {
    public:
        uint32_t delay;
        long double begin;
        unsigned char message;
        unsigned char message_ex;
        uint32_t data_size;
        std::shared_ptr <unsigned char> message_data;

        midi_track* track;

        midi_event()
        {
            delay = 0;
            begin = 0;
            message = 0;
            message_ex = 0;
            data_size = 0;
        };

        void set_context();

        char* env_read(char* s)
        {
            char* x = s;
            {
                int i = 0;
                delay = 0;
                do
                {
                    delay <<= 7;
                    i = *s;
                    delay |= i & 0x7F;
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
                    int i = 0;
                    message_ex = *s;
                    s++;
                    data_size = 0;
                    do
                    {
                        data_size <<= 7;
                        i = *s;
                        data_size |= i & 0x7F;
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
            case release:
            case push:
            case touch:
            case controller:
            case slip:
                length = 2;
                break;
            case instrument:
            case pressure:
                length = 1;
                break;
            case system:
                length = 0;
                break;
            }
            return length;
        }

        void print();
        std::string data_str();

        bool operator <(const midi_event& in)
        {
            if (begin < in.begin)
                return true;
            else
                return false;
        }
        bool operator >(const midi_event& in)
        {
            if (begin > in.begin)
                return true;
            else
                return false;
        }

        midi_event_notes to_note()
        {
            unsigned char M = message & 0xf0;
            if ((M == release) || (M == push) || (M == touch))
            {
                unsigned char* s = message_data.get();
                return midi_event_notes(s[0], s[1]);
            }
            else
                return midi_event_notes();
        }

        uint32_t sys_get_time()
        {
            uint32_t time = 0;

            if (message_ex == 0x51)
            {
                unsigned char* s = message_data.get();
                time |= s[0]; time <<= 8;
                time |= s[1]; time <<= 8;
                time |= s[2];
            }
            return time;
        }
    };

    class midi_track
    {
    public:
        midi_file* file;
        char m_magic[4];
        uint32_t m_seclen;
        //std::vector<MidiMessage> m_midi_messages;
        std::vector<midi_event> events;
        midi_track()
        {
        };
        void set_context();

        void print();
    };

    class midi_file_interface
    {
    private:
        int size;// 文件总大小 file size
        int pos;// 当前读到的位置 pos
        std::ifstream F;//文件对象 file
        std::shared_ptr<char> buf;//缓冲区 buf
    public:

        MidiReader* reader;   //上文

        midi_file_interface()
        {
            size = 0;
            pos = 0;
        }
        midi_file_interface(std::string path)
        {
            size = 0;
            pos = 0;
            read(path);
        }
        ~midi_file_interface()
        {
            if (F.is_open())
            {
                F.close();
            }
        }

        //读取文件到缓冲区，然后关闭文件
        bool read(std::string file_path);

        //读取数据
        bool _get_byte(char* data_buf, int byte_num, bool move);//从文件中读取指定字节大小的内容
        bool get(void* addr, size_t len, bool is_char, bool move);
        bool get_str(std::string& str, size_t len);


        int get_pos()//获取当前读取所在位置
        {
            return pos;
        }
        void set_pos(int _pos)//设置当前读取所在位置
        {
            pos = _pos;
        }
        void move_pos(int move)//移动当前读取所在位置
        {
            int _pos = pos + move;
            if ((_pos >= 0) && (_pos <= size))
                set_pos(_pos);
        }
        char* get_buf()
        {
            return buf.get() + pos;
        }
    };

    class midi_file
    {
    private:
        bool read_header();
        bool read_tracks();


        void get_event_times(void);
        void set_event_times(void);
        void get_notes(void);
        void get_max_notes(void);
    public:
        MidiHeader header;
        std::vector<midi_track> tracks;// [header.m_ntracks];
        midi_file_interface MF_interface;
        std::vector<midi_event> time_events;
        MidiReader* reader;
        std::vector < midi_note> notes;
        int max_notes_same_time = 0;

        bool read(std::string path);

        //屏幕输出 print
        void print_header();
        void print_tracks();
        void print_notes();

        void print();

        midi_file()
        {
        }

        void set_context();
    };

    class midi_wav_maker
    {
    private:
        struct {
            char RIFF[4];    //头部分那个RIFF
            long int size0;//存的是后面所有文件大小
            char WAVE[4];
            char FMT[4];
            long int size1;//存的是fmt保存的大小，包含这之后，data前面几个，共16个
            short int fmttag;
            short int channel;
            long int samplespersec;//每秒采样数，用的是sampleRate
            long int bytepersec;
            short int blockalign;
            short int bitpersamples;
            char DATA[4];
            long int size2;//声音文件大小。

        }head;

        const int sampleRate = 44100;
        const double PI = 3.1415926535897932384626433832795;
        float press_point[1000];

        int buf_note(float* s, midi_note* note, int channel);

        float get_press(int i)//计算每一毫秒的衰减
        {
            if (i < 1000)
                return press_point[i];
            else
                return 0;
        }
    public:

        MidiReader* reader;

        midi_wav_maker();

        void set_context();

        void output(std::string PATH);


    };
}

class MidiReader
{
private:
    // private:
    bool have_read = false;
public:
    _midi_reader::midi_file midi;
    _midi_reader::midi_wav_maker wav_maker;
    MidiReader()
    {
    
    }
    MidiReader(std::string file_path)
    {
        read(file_path);
    };

    void set_context();
    bool read(std::string path);
    void print();
    bool make_wav(std::string PATH);
};

#endif