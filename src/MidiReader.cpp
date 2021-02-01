#include "MidiReader.h"
#include "error_def.h"
#include <string.h>

void bzero(void* ptr, size_t sz)
{
    memset(ptr, 0, sz);
}

// 大小端转换
void EndianSwap(char *pData, int length)
{
    if (length <= 1) return;
    for (int i = 0; i <= length / 2 - 1; ++i)
    {
        std::swap(pData[i], pData[length - i - 1]);
    }
}

//文件的打开与关闭
//打开文件
bool MidiReader_file_interface::read(std::string file_path)
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
bool MidiReader_file_interface::_get_byte(char* data_buf,int byte_num,bool move= true)
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


bool MidiReader_file_interface::get(void* addr, size_t len, bool is_char=false, bool move = true)
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

bool MidiReader_file_interface::get_str(std::string& str, size_t len)
{
    str.resize(len);                                //改变str的大小
    if (_get_byte(&str[0],len))                    //读取len个字节
    {                                               //
        return true;                                //
    }                                               //
    //PRINT_ERROR(ERROR_READ_STRING);                  //
    return false;                               //
}


MidiReader::MidiReader()                    //
{                                           //
};                                          //

MidiReader::MidiReader(std::string file_path)
{
    read(file_path);
};

//析构函数
MidiReader::~MidiReader()
{
}


bool MidiReader::read(std::string path)
{
    MF_interface.read(path);
    if (read_header() && read_tracks())        //
    {                                          //
        return true;                           //
    }
    else
        return false;
}

bool MidiReader::read_header()
{
    if (MF_interface.get(&midi.header.m_magic, 4, true) &&    //
        MF_interface.get(addr_and_size(midi.header.m_seclen)) &&           //
        MF_interface.get(addr_and_size(midi.header.m_format)) &&           //
        MF_interface.get(addr_and_size(midi.header.m_ntracks)) &&         //
        MF_interface.get(addr_and_size(midi.header.m_tickdiv)))           //
    {                                                            //
        //is_read_header_ok = true;                                //
        return true;                                             //
    }                                                            //
    return false;                                                //
}

bool MidiReader::read_tracks()
{
    bool have;
    do
    {
        midi.tracks.push_back(MidiTrack());
        MidiTrack& track = midi.tracks.back();

        have = MF_interface.get(&track.m_magic, 4, true)&& MF_interface.get(addr_and_size(track.m_seclen));
        if (have)
        {
            int remaining = track.m_seclen;
            char* BUF = MF_interface.get_buf();
            char* _BUF = BUF;
            do
            {
                track.events.push_back(midi_event());
                midi_event& E = track.events.back();
                BUF = E.env_read(BUF);
                if ((E.message == 0xff) && (E.message_ex == 0x2f))
                    break;
            } while (remaining > BUF - _BUF);
            MF_interface.move_pos(remaining);
        }
        else
        {
            break;
        }
    } while (1);
    midi.tracks.pop_back();
    if(midi.tracks.size()==0)
        return false;
    else
        return true;
    
}
/*

bool MidiReader::read_messages(MidiMessage &message)
{
    if (!read_delta_time(message.m_dtime))return false;
    if (!MF_interface.get(addr_and_size(message.m_status)))return false;
    char &m_status = message.m_status;
    char lastStatus = 0;


    if (m_status & 0x80)
        lastStatus = m_status;
    else;
        //fs.seekg(-1, fs.tellg());  // FSeek(FTell() - 1);

    message.m_channel = (lastStatus & 0x0f);
    //if(lastStatus == -1)
    //{
    //    if (!read_meta_event(message.meta_event))
    //        return false;
    //}
    //switch ((lastStatus & 0xf0))
    //{
    //case 0x80://松开音符
    //    break;
    //case 0x90://按下音符
    //    break;
    //case 0xA0://触后音符
    //    break;
    //case 0xB0://控制器
    //    break;
    //case 0xC0://改变乐器
    //    break;
    //case 0xD0://触后通道
    //    break;
    //case 0xE0://滑音
    //    break;
    //case 0xF0://系统码
    //    break;
    //
    //}


    if ((lastStatus & 0xf0) == 0x80)
    {
        if (!MF_interface.get(&message.note_off_event, 2, true))return false;
    }
    else if ((lastStatus & 0xf0) == 0x90)
    {
        if (!MF_interface.get(&message.note_on_event, 2, true))return false;
    }
    else if ((lastStatus & 0xf0) == 0xA0)
    {
        if (!MF_interface.get(&message.note_pressure_event, 2, true))return false;
    }
    else if ((lastStatus & 0xf0) == 0xB0)
    {
        if (!MF_interface.get(&message.controller_event, 2, true))return false;
    }
    else if ((lastStatus & 0xf0) == 0xC0)
    {
        if (!MF_interface.get(&message.program_event, 1, true))return false;
    }
    else if ((lastStatus & 0xf0) == 0xD0)
    {
        if (!MF_interface.get(&message.channel_pressure_event, 1, true))return false;
    }
    else if ((lastStatus & 0xf0) == 0xE0)
    {
        if (!MF_interface.get(&message.pitch_bend_event, 2, true))return false;
    }
    else if (lastStatus == -1)
    {
        if (!read_meta_event(message.meta_event))return false;
    }
    else if ((lastStatus & 0xf0) == 0xF0)
    {
        if (!read_sysex_event(message.sysex_event))return false;
    }
    return true;
}
*/
/*
bool MidiReader::read_delta_time(DeltaTime &dt)
{
    uint32_t &total = dt.total;

    MF_interface.get(addr_and_size(dt.t0));
    total += dt.t0 & 0x7f;
    if (!(dt.t0 & 0x80)) return true;
    MF_interface.get(addr_and_size(dt.t1));
    total <<= 7;
    total += dt.t1 & 0x7f;
    if (!(dt.t1 & 0x80)) return true;
    MF_interface.get(addr_and_size(dt.t2));
    total <<= 7;
    total += dt.t2 & 0x7f;
    if (!(dt.t2 & 0x80)) return true;
    MF_interface.get(addr_and_size(dt.t3));
    total <<= 7;
    total += dt.t3 & 0x7f;
    if (!(dt.t3 & 0x80)) return true;
    return false;
}
*/
/*
bool MidiReader::read_meta_event(MetaEvent &me)
{
    MF_interface.get(&me.m_type, 1);
    read_delta_time(me.m_length);
    Type &m_type = me.m_type;
    DeltaTime &m_length = me.m_length;
    if (m_type == META_SEQUENCE_NUM)
    {
        MF_interface.get(addr_and_size(me.m_seqNum));
    }
    else if (m_type == META_TEXT)
    {
        MF_interface.get_str(me.m_text, m_length.total);
    }
    else if (m_type == META_COPYRIGHT)
    {
        MF_interface.get_str(me.m_copyright, m_length.total);
    }
    else if (m_type == META_SEQUENCE_NAME)
    {
        MF_interface.get_str(me.m_name, m_length.total);
    }
    else if (m_type == META_INSTRUMENT_NAME)
    {
        MF_interface.get_str(me.m_name, m_length.total);
    }
    else if (m_type == META_LYRIC)
    {
        MF_interface.get_str(me.m_lyric, m_length.total);
    }
    else if (m_type == META_MARKER)
    {
        MF_interface.get_str(me.m_marker, m_length.total);
    }
    else if (m_type == META_CUE_POINT)
    {
        MF_interface.get_str(me.m_cuePoint, m_length.total);
    }
    else if (m_type == META_PROGRAM_NAME)
    {
        MF_interface.get_str(me.m_programName, m_length.total);
    }
    else if (m_type == META_DEVICE_NAME)
    {
        MF_interface.get_str(me.m_deviceName, m_length.total);
    }
    else if (m_type == META_MIDI_CHANNEL_PREFIX)
    {
        MF_interface.get(addr_and_size(me.m_channelPrefix));
    }
    else if (m_type == META_MIDI_PORT)
    {
        MF_interface.get(addr_and_size(me.m_port));
    }
    else if (m_type == META_END_OF_TRACK)
    {
    }
    else if (m_type == META_TEMPO)
    {
        uint32_t m_usecPerQuarterNote;
        MF_interface.get(&m_usecPerQuarterNote, 3);
        me.m_usecPerQuarterNote = m_usecPerQuarterNote;
        me.m_bpm = 60000000 / m_usecPerQuarterNote;
        //fs.seekg(-1, fs.tellg());
    }
    else if (m_type == META_SMPTE_OFFSET)
    {
        MF_interface.get(addr_and_size(me.m_hours));
        MF_interface.get(addr_and_size(me.m_mins));
        MF_interface.get(addr_and_size(me.m_secs));
        MF_interface.get(addr_and_size(me.m_fps));
        MF_interface.get(addr_and_size(me.m_fracFrames));
    }
    else if (m_type == META_TIME_SIGNATURE)
    {
        MF_interface.get(addr_and_size(me.m_numerator));
        MF_interface.get(addr_and_size(me.m_denominator));
        MF_interface.get(addr_and_size(me.m_clocksPerClick));
        MF_interface.get(addr_and_size(me.m_32ndPer4th));
    }
    else if (m_type == META_KEY_SIGNATURE)
    {
        MF_interface.get(addr_and_size(me.m_flatsSharps));
        MF_interface.get(addr_and_size(me.m_majorMinor));
    }
    else
    {
        MF_interface.get_str(me.m_data, m_length.total);
    }
    return true;
}
*/
/*
bool MidiReader::read_sysex_event(SysexEvent &se)
{
    read_delta_time(se.m_length);
    MF_interface.get_str(se.m_message, se.m_length.total);
    return true;
}
*/
void MidiReader::print_header()
{
    
    char s[5];
    int i;
    for (i = 0; i < 4; i++)
    {
        s[i] = midi.header.m_magic[i];
    }
    s[4] = '\0';
    std::cout << "header : \n" <<
        "m_magic = " << s << "\t" <<
        "m_seclen = " << midi.header.m_seclen << "\t" <<
        "m_format = " << midi.header.m_format << "\t" <<
        "m_ntracks = " << midi.header.m_ntracks << "\t" <<
        "m_tickdiv = " << midi.header.m_tickdiv << "\n";
}

void MidiReader::print_tracks()
{
    std::vector<MidiTrack>::iterator T;
    for (T = midi.tracks.begin(); T != midi.tracks.end(); T++)
    {
        std::cout << "--->size:" << T->m_seclen << std::endl;
        T->print_env();
    }
}

void MidiReader::print()
{
    print_header();
    print_tracks();
}



