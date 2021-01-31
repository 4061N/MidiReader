#include "MidiReader.h"
#include "error_def.h"
#include <string.h>

void bzero(void* ptr, size_t sz)
{
    memset(ptr, 0, sz);
}

// ��С��ת��
void EndianSwap(char *pData, int length)
{
    if (length <= 1) return;
    for (int i = 0; i <= length / 2 - 1; ++i)
    {
        std::swap(pData[i], pData[length - i - 1]);
    }
}

/*
//���ļ�
bool MidiReader::open_file(std::string file_path)
{
    fs.open(file_path, std::ios_base::in | std::ios_base::binary);  //�Զ�ȡģʽ���ļ�
    if (!is_file_opened())                                          //���ļ���ʧ��
    {                                                               //
        PRINT_ERROR(ERROR_READ_FILE)                                //��ӡ������Ϣ��ERROR -1 ���ļ�ʧ��
        return false;                                               //
    }                                                               //
    init();                                                         //�����ļ���С�ͻ�����
    return true;                                                    //
}*/

//�ļ��Ĵ���ر�
//���ļ�
bool MidiReader_file_interface::read(std::string file_path)
{
    pos = 0;                                                            //��ǰλ��=0
    F.open(file_path, std::ios_base::in | std::ios_base::binary);       //�Զ�ȡģʽ���ļ�
                                                                        //
    if (F.is_open())                                                    //�Ƿ���ļ�
    {                                                                   //
                                                                        //
        {                                                               //
            F.seekg(0, F.end);                                          //���ļ�ĩβ 
            size = F.tellg();                                           //��ȡ�ļ���С 
            F.seekg(F.beg);                                             //�ص��ļ���ͷ
        }                                                               //
                                                                        //
                                                                        //
        buf.reset(new char[size]);                                      //���û�������С
        F.read(buf.get(), size);                                        //���ļ��ж�ȡ���ݵ������� 
        F.close();                                                      //�ر��ļ�
        return true;                                                    //
    }                                                                   //
    else                                                                //��ʧ��
    {                                                                   //
        size = 0;                                                       //�ļ���С=0
        return false;                                                   //
    }                                                                   //
}

//�ļ��Ķ�ȡ
//�ӻ������ж�ȡָ���ֽڴ�С������
bool MidiReader_file_interface::_get_byte(char* data_buf,int byte_num)
{
    if ((pos + byte_num) > size)
    {
        PRINT_ERROR(ERROR_OVER_RANGE)        //��ӡ������Ϣ��ERROR -3 ��ȡλ�ó�����С
        return false;
    }
    memcpy_s(data_buf, byte_num, buf.get()+pos, byte_num);
    pos += byte_num;
    return true;
}


bool MidiReader_file_interface::get(void* addr, size_t len, bool is_char=false)
{
    if (_get_byte((char*)addr,len))
    {
        if (!is_char)
        {
            EndianSwap((char*)addr, len);   //��С��ת��
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
    str.resize(len);                                //�ı�str�Ĵ�С
    if (_get_byte(&str[0],len))                    //��ȡlen���ֽ�
    {                                               //
        return true;                                //
    }                                               //
    //PRINT_ERROR(ERROR_READ_STRING);                  //
    return false;                               //
}


//��ջ����� clean data buffer
void MidiReader::buff_clean()
{
    bzero(static_cast<void*>(buff.get()), file_size);
}
//�����ļ���С�ͻ�����
void MidiReader::init()                     //
{                                           //
    fs.seekg(0, fs.end);                    //�ļ�ĩβ
    if (file_size < fs.tellg())             //�����ô�СС���ļ�
    {                                       //
        file_size = (int)fs.tellg();        //�����ļ���С
        buff.reset(new char[file_size]);    //���û�������С
    }                                       //
    fs.seekg(fs.beg);                       //�ļ���ͷ
}

MidiReader::MidiReader()                    //
{                                           //
};                                          //

MidiReader::MidiReader(std::string file_path)
{
    read(file_path);
};

//��������
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

/*
//���ļ��ж�ȡָ���ֽڴ�С������
bool MidiReader::read(int byte_num)
{
    if (current_pos + byte_num > file_size)  //
    {                                        //
        PRINT_ERROR(ERROR_OVER_RANGE);        //��ӡ������Ϣ��ERROR -3 ��ȡλ�ó����ļ���С
        return false;                        //
    }                                        //
    //buff_clean();                          //
    fs.read(buff.get(), byte_num);           //���ļ��ж�ȡ���ݵ�������
    current_pos += byte_num;                 //��ǰ��ȡλ��ˢ��
    return true;                             //
}


template<typename T>


bool MidiReader::read_var(const T &t, void* addr, size_t len, bool is_char)
{
    size_t sz = len ? len : sizeof(t);                         //
    if (read(sz))                                              //��ȡsz���ֽ�
    {                                                          //
        memcpy_s(addr, sz, buff.get(), sz);                    //
        // �ļ�������Ĭ���Ǵ�� Ҫת����С��                   //
        if (!is_char)EndianSwap((char *)&t, len ? len : sz);   //��С��ת��
        return true;                                           //
    }                                                          //
    else                                                       //
    {                                                          //
        PRINT_ERROR(ERROR_READ_HEADER);                         //��ӡ������Ϣ��ERROR -4 ��ȡ�ļ�ͷ����
        return false;                                           //
    }                                                          //
}
bool MidiReader::read_str(std::string &str, size_t len)
{
    str.resize(len);                                //�ı�str�Ĵ�С
    if (read(len))                                  //��ȡlen���ֽ�
    {                                               //
        memcpy_s(&str[0], len, buff.get(), len);    //����ȡ�������ݿ�����str��
        return true;                                //
    }                                               //
    PRINT_ERROR(ERROR_READ_STRING)                  //
    return false;                                   //
}

bool MidiReader::read_file(MidiFile &file)
{
    read_header(file.header);                                 //
    if (is_read_header_ok && read_tracks(file.tracks))        //
    {                                                         //
        return true;                                          //
    }                                                         //
    return false;                                             //
}*/

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
    /*MF_interface.get(&midi.tracks.m_magic, 4, true);
    MF_interface.get(addr_and_size(midi.tracks.m_seclen));
    midi.tracks.m_seclen = 0;*/
    
    if (MF_interface.get(&midi.tracks.m_magic, 4, true) &&                        //
        MF_interface.get(addr_and_size(midi.tracks.m_seclen))                     //
        )    //                                                                   //
    {                                                                             //
                                                                                  //
        int remaining = midi.tracks.m_seclen;                                     //
        int init_pos = MF_interface.get_pos();                                    //
        do {                                                                      //
            midi.tracks.m_midi_messages.push_back(MidiMessage());                 //
            MidiMessage &message = midi.tracks.m_midi_messages.back();            //
            if (!read_messages(message)) return false;                            //
            //remaining -= sizeof(message);                                       //
        } while (remaining < MF_interface.get_pos() - init_pos);                  //
        return true;                                                              //
    }                                                                             //
    return false;                                                                 //
}


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
    /*if(lastStatus == -1)
    {
        if (!read_meta_event(message.meta_event))
            return false;
    }
    switch ((lastStatus & 0xf0))
    {
    case 0x80://�ɿ�����
        break;
    case 0x90://��������
        break;
    case 0xA0://��������
        break;
    case 0xB0://������
        break;
    case 0xC0://�ı�����
        break;
    case 0xD0://����ͨ��
        break;
    case 0xE0://����
        break;
    case 0xF0://ϵͳ��
        break;

    }*/
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


bool MidiReader::read_sysex_event(SysexEvent &se)
{
    read_delta_time(se.m_length);
    MF_interface.get_str(se.m_message, se.m_length.total);
    return true;
}

void MidiReader::print_header(const MidiHeader &header)
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

void MidiReader::print_tracks(const MidiTrack &tracks)
{
    std::cout << "track : \n" <<
        "m_magic = " << tracks.m_magic << "\t" <<
        "m_seclen = " << tracks.m_seclen << "\t";
    int count = 1;
    for (const auto &msg : tracks.m_midi_messages)
    {
        std::cout << "msg" << count++ << " : \n" <<
            msg.m_dtime << "\t" <<
            "m_status = " << msg.m_status << "\t" <<
            "lastStatus = " << msg.lastStatus << "\n";
    }
}

void MidiReader::print_file(const MidiFile &file)
{
    print_header(file.header);
    print_tracks(file.tracks);
}



