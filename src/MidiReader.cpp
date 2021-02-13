#include "MidiReader.h"
#include "error_def.h"
#include <string.h>
#include <algorithm>
#include <list>

/**
void bzero(void* ptr, size_t sz)
{
    memset(ptr, 0, sz);
}*/

// 大小端转换
void EndianSwap(char *pData, int length)
{
    if (length <= 1) return;
    for (int i = 0; i <= length / 2 - 1; ++i)
    {
        std::swap(pData[i], pData[length - i - 1]);
    }
}

//读取文件到缓冲区，然后关闭文件
bool midi_file_interface::read(std::string file_path)
{
    pos = 0;                                                            //当前位置=0
    F.open(file_path, std::ios_base::in | std::ios_base::binary);       //以读取模式打开文件
                                                                        //
    if (F.is_open())                                                    //是否打开文件
    {                                                                   //
                                                                        //
        {                                                               //
            F.seekg(0, F.end);                                          //到文件末尾 
            size = F.tellg();                                           //获取文件大小 
            F.seekg(F.beg);                                             //回到文件开头
        }                                                               //
                                                                        //
                                                                        //
        buf.reset(new char[size]);                                      //重置缓冲区大小
        F.read(buf.get(), size);                                        //从文件中读取数据到缓冲区 
        F.close();                                                      //关闭文件
        return true;                                                    //
    }                                                                   //
    else                                                                //打开失败
    {                                                                   //
        size = 0;                                                       //文件大小=0
        return false;                                                   //
    }                                                                   //
}

//文件的读取
//从缓冲区中读取指定字节大小的内容
bool midi_file_interface::_get_byte(char* data_buf,int byte_num,bool move= true)
{
    if ((pos + byte_num) > size)
    {
        return false;
    }
    memcpy_s(data_buf, byte_num, buf.get()+pos, byte_num);
    if(move)
        pos += byte_num;
    return true;
}


bool midi_file_interface::get(void* addr, size_t len, bool is_char=false, bool move = true)
{
    if (_get_byte((char*)addr,len,move))
    {
        if (!is_char)
        {
            EndianSwap((char*)addr, len);   //大小端转换
        }
        return true;
    }
    else
    {
        return false;
    }
}
#define addr_and_size(var) &var,sizeof(var)

bool midi_file_interface::get_str(std::string& str, size_t len)
{
    str.resize(len);                                //改变str的大小
    if (_get_byte(&str[0],len))                    //读取len个字节
    {                                               //
        return true;                                //
    }                                               //
    return false;                               //
}

void  midi_file::get_event_times(void)
{
    time_events.clear();
    for (auto T: tracks)
    {
        unsigned long long time = 0;
        for (auto E : T.events)
        {
            time += E.delay;
            E.int_begin = time;
        }
    }
    for (auto T : tracks)
    {
        for (auto E : T.events)
        {
            if (E.message_ex == 0x51)
            {
                time_events.push_back(E);
            }
        }
    }
    sort(time_events.begin(), time_events.end());

}

void midi_file::set_event_times(void)
{
    
    std::vector<midi_track>::iterator T;
    std::vector <midi_event> ::iterator E,times;
    int div= header.m_tickdiv;
    for (T = tracks.begin(); T != tracks.end(); T++)
    {
        times = time_events.begin();
        long double time = 0 , qn = times->sys_get_time();
        
        for (E = T->events.begin(); E != T->events.end(); E++)
        {
            if (*E > *times)
            {
                if (times+1 != time_events.end())
                {
                    times++;
                    qn = times->sys_get_time();
                    
                }
            }
            time += (E->delay * qn) / div;
            E->begin = time;
        }
    }
}

void midi_file::get_notes(void)
{
    notes.clear();  
    //std::vector <midi_track>::iterator T;
    std::vector <midi_event> ::iterator E,i;
    for (auto T:tracks)
    {
        for (E = T.events.begin(); E != T.events.end(); E++)
        {
            if (E->message == midi_reader::push)
            {
                unsigned char note_message = E->message_data.get()[0];
                bool find = false;
                for (i = E + 1; i != T.events.end(); i++)
                {
                    if (i->message == midi_reader::release)
                    {
                        if (i->message_data.get()[0] == note_message)
                        {
                            //midi_note _note(E->begin, j->begin, note_message, E->message_data.get()[1]);
                            notes.push_back(midi_note(E->begin/1000, i->begin / 1000, note_message, E->message_data.get()[1]));
                            find = true;
                            break;
                        }
                    }
                }
            }
        }
    }
    sort(notes.begin(), notes.end());

}


bool midi_file::read_header()
{
    if (MF_interface.get(&header.m_magic, 4, true) &&    //
        MF_interface.get(addr_and_size(header.m_seclen)) &&           //
        MF_interface.get(addr_and_size(header.m_format)) &&           //
        MF_interface.get(addr_and_size(header.m_ntracks)) &&         //
        MF_interface.get(addr_and_size(header.m_tickdiv)))           //
    {                                                            //
        //is_read_header_ok = true;                                //
        return true;                                             //
    }                                                            //
    return false;                                                //
}

bool midi_file::read_tracks()
{
    bool have;
    unsigned long long time=0;
    do
    {
        tracks.push_back(midi_track());
        midi_track& track = tracks.back();
        

        have = MF_interface.get(&track.m_magic, 4, true) && MF_interface.get(addr_and_size(track.m_seclen));
        if (have)
        {
            int remaining = track.m_seclen;
            int begin = MF_interface.get_pos();
            do
            {
                char * BUF = MF_interface.get_buf();
                track.events.push_back(midi_event());
                midi_event& E = track.events.back();
                MF_interface.move_pos(E.env_read(BUF)-BUF);
                if ((E.message == 0xff) && (E.message_ex == 0x2f))
                    break;
            } while (remaining > MF_interface.get_pos()-begin);
            MF_interface.move_pos(remaining);
        }
        else
        {
            break;
        }
    } while (1);
    tracks.pop_back();
    if (tracks.size() == 0)
        return false;
    else
        return true;

}

bool midi_file::read(std::string path)
{
    MF_interface.read(path);
    if (read_header() && read_tracks())        //
    {                                          //
        get_event_times();
        set_event_times();
        get_notes();
        return true;                           //
    }
    else
        return false;
}

bool MidiReader::read(std::string path)
{
    bool d = midi.read(path);
    if (d)
    {
        set_context();
        have_read = true;
    }
    else
    {
        have_read = false;
    }
    return d;
}


bool MidiReader::make_wav(std::string PATH)
{
    if (have_read)
    {
        wav_maker.output(PATH);
        return true;
    }
    return false;
}


void midi_event::set_context()
{

}

void midi_track::set_context()
{
    for (auto E: events)
    {
        E.track = this;
        E.set_context();
    }
}

void midi_file::set_context()
{
    for (auto T: tracks)
    {
        T.file = this;
        T.set_context();
    }
}

void midi_wav_maker::set_context()
{

}

void MidiReader::set_context()
{
    midi.reader = this;
    wav_maker.reader = this;
    midi.set_context();
    wav_maker.set_context();
}

std::string midi_event::data_str()
{
    unsigned char* d = message_data.get();
    unsigned char* s = new unsigned char[data_size + 1];
    int i;
    for (i = 0; i < data_size; i++)
        s[i] = d[i];
    s[data_size] = '\0';
    std::string str((char *)s);
    delete s;
    return str;
}



void midi_note::print()
{
    printf("开始时间：%12.3f ms\t", begin);
    printf("结束时间：%12.3f ms\t", end);
    printf("持续时间：%12.3f ms\t", time);
    printf("频率：%12.3f Hz \t", frequency);
    //printf("力度：%d\t", press);
    midi_event_notes(note_octave, press).print();
}

void midi_event_notes::print()
{
    printf(("音符：%d" + notes_str() + " \t").c_str(), octave);
    //printf("八度：%d \t", octave);
    printf("力度：%d \t", press);
}
void midi_event::print()
{ 
    printf("时间:%12.3f ms\t", begin /1000);
    printf("音轨:%d\t", message & 0x0f);
    unsigned char* d = message_data.get();
    switch (message & (0xf0))
    {
    case midi_reader::release:
    {
        printf("-松开按键\t");
        to_note().print();
        printf("\n");
    }
    break;
    case midi_reader::push:
    {
        printf("+按下按键\t");
        to_note().print();
        printf("\n");
    }
    break;
    case midi_reader::touch:
    {
        printf("触后音符\t");
        to_note().print();
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
        printf("设定乐器\t");
        printf("乐器号码：%d\t", d[0]);
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
        printf("系统信息：");
        switch (message_ex)
        {
        case 0x00://FF 00 设置轨道音序
            printf("设置轨道音序\t");
            break;
        case 0x01://FF 01 文本信息
            printf("文本信息:%s\t", data_str().c_str());
            break;
        case 0x02://FF 02 版权信息
            printf("版权信息:%s\t", data_str().c_str());
            break;
        case 0x03://FF 03 音轨名称或歌曲名称
            printf("名称:%s\t", data_str().c_str());
            break;
        case 0x04://FF 04 乐器
            printf("乐器\t", data_str().c_str());
            break;
        case 0x05://FF 05 歌词
            printf("歌词\t", data_str().c_str());
            break;
        case 0x06://FF 06 标记
            printf("标记\t", data_str().c_str());
            break;
        case 0x07://FF 07 提示点/注释
            printf("提示点/注释\t", data_str().c_str());
            break;
        case 0x20://FF 20 MIDI通道
            printf("MIDI通道:%d\t",d[0]);
            break;
        case 0x21://FF 21 MIDI端口
            printf("MIDI端口:%d\t", d[0]);
            break;
        case 0x2F://FF 2F MTrk结束标记
            printf("MTrk结束标记\t");
            break;
        case 0x51://FF 51 速度
            printf("设置四分音符长度%d us\t", sys_get_time());
            break;
        case 0x54://FF 54 SMPTE偏移     FF 54 05 hr mn se fr ff
            printf("SMPTE偏移\t");
            break;
        case 0x58://FF 58 时间签名      FF 58 04 nn dd cc bb
            printf("节拍信息\t");
            break;
        case 0x59://FF 59 谱号信息      FF 59 02 sf mi
        {
            printf("谱号信息:");
            if (d[0] & 0x80)
                printf("降%d",0x10 -(d[0] & 0x0f));
            else
                printf("升%d",(d[0] & 0x0f));

            if (d[1])
                printf("小调");
            else
                printf("大调");
        }
            break;
        case 0x7F://FF 7F 专有/其他数据
            printf("专有/其他数据\t");
            break;
        }
        printf("\n");
    }
    break;
    }
}

void midi_track::print()
{
    std::cout << "--->events:" << events.size() << std::endl;
    for (auto E: events)
    {
        E.print();
    }

}

void midi_file::print_header()
{
    
    char s[5];
    int i;
    for (i = 0; i < 4; i++)
    {
        s[i] = header.m_magic[i];
    }
    s[4] = '\0';
    std::cout << "header : \n" <<
        "m_magic = " << s << "\t" <<
        "m_seclen = " << header.m_seclen << "\t" <<
        "m_format = " << header.m_format << "\t" <<
        "m_ntracks = " << header.m_ntracks << "\t" <<
        "m_tickdiv = " << header.m_tickdiv << "\n";
}

void midi_file::print_tracks()
{
    for (auto T: tracks)
    {
        std::cout << "--->size:" << T.m_seclen << std::endl;
        T.print();
    }
}

void midi_file::print_notes()
{
    for (auto N : notes)
    {
        N.print();
        printf("\n");
    }
}

void midi_file::print()
{
    print_header();
    print_tracks();
    print_notes();
}

void MidiReader::print()
{
    midi.print();
}

int midi_wav_maker::buf_note(int16_t* s, midi_note* note, int channel)
{
    int SAMPLE_NUM = sampleRate * note->time / 1000; //采集样本总数
    int AUDIO_CYCLE = sampleRate / note->frequency; //一个正弦波采集样本个数
    int i;
    float amp = (float)note->press / channel;
    float press = 0;
    s = s + (int)((float)sampleRate * note->begin / 1000);
    for (i = 0; i < SAMPLE_NUM; i++)
    {
        if ((i % (sampleRate / 1000)) == 0)
        {
            press = amp * get_press(i / (sampleRate / 1000));
        }

        s[i] = s[i] + (int16_t)(((float)press * sin(2 * PI * i / AUDIO_CYCLE) * 127));//使用sin函数，初始点是0
    }
    return SAMPLE_NUM;
}

midi_wav_maker::midi_wav_maker()
{
    head =
    {
        {'R','I','F','F'},
        0, //文件长度=该字段+ 8
        {'W','A','V','E'},
        {'f','m','t',' '},
        16, //格式长度，在此之前成员的大小
        1,   //编码格式为PCM
        1, //声道
        sampleRate, //采样频率
        sampleRate * 2,// 数据传输速率
        2, //数据块对齐单位(一个sample所占字节数)
        16,//采样位数(数度)
        {'d','a','t','a'},
        0
    };//采样数据的大小

    for (int i = 0; i < 10; i++)
    {
        press_point[i] = i * (1 / 10);
    }
    float s = 1;
    for (int i = 10; i < 2000; i++)
    {
        s = press_point[i] = s * 0.995;
    }
}

void midi_wav_maker::output(std::string PATH)
{
    long SAMPLE_NUM = sampleRate* reader->midi.notes.back().end / 1000 ; //采集样本总数
    std::shared_ptr<int16_t>body;
    body.reset(new int16_t[SAMPLE_NUM]);
    int16_t* s = body.get();

    int channel = reader->midi.tracks.size() - 1;

    for (long i = 0; i < SAMPLE_NUM; i++)
    {
        s[i] = 0;
    }

    for (auto i : reader->midi.notes)
    {
        buf_note(s, &i, channel);
    }
    head.size2 = SAMPLE_NUM * 2;
    head.size0 = head.size2 + 16 + 8;

    std::ofstream ocout;
    ocout.open(PATH, std::ios::out | std::ios::binary);//打开（不存在时生成）
    ocout.write((char*)&head, sizeof head);//将文件头部分写进文件
    ocout.write((char*)body.get(), SAMPLE_NUM * 2);//将数据文件写入程序
    ocout.close();//关闭文件
}

