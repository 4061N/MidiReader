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

// ��С��ת��
void EndianSwap(char *pData, int length)
{
    if (length <= 1) return;
    for (int i = 0; i <= length / 2 - 1; ++i)
    {
        std::swap(pData[i], pData[length - i - 1]);
    }
}

//��ȡ�ļ�����������Ȼ��ر��ļ�
bool midi_file_interface::read(std::string file_path)
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

bool midi_file_interface::get_str(std::string& str, size_t len)
{
    str.resize(len);                                //�ı�str�Ĵ�С
    if (_get_byte(&str[0],len))                    //��ȡlen���ֽ�
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
    printf("��ʼʱ�䣺%12.3f ms\t", begin);
    printf("����ʱ�䣺%12.3f ms\t", end);
    printf("����ʱ�䣺%12.3f ms\t", time);
    printf("Ƶ�ʣ�%12.3f Hz \t", frequency);
    //printf("���ȣ�%d\t", press);
    midi_event_notes(note_octave, press).print();
}

void midi_event_notes::print()
{
    printf(("������%d" + notes_str() + " \t").c_str(), octave);
    //printf("�˶ȣ�%d \t", octave);
    printf("���ȣ�%d \t", press);
}
void midi_event::print()
{ 
    printf("ʱ��:%12.3f ms\t", begin /1000);
    printf("����:%d\t", message & 0x0f);
    unsigned char* d = message_data.get();
    switch (message & (0xf0))
    {
    case midi_reader::release:
    {
        printf("-�ɿ�����\t");
        to_note().print();
        printf("\n");
    }
    break;
    case midi_reader::push:
    {
        printf("+���°���\t");
        to_note().print();
        printf("\n");
    }
    break;
    case midi_reader::touch:
    {
        printf("��������\t");
        to_note().print();
        printf("\n");
    }
    break;
    case midi_reader::controller:
    {
        printf("�ı������\t");
        printf("���������룺%d\t", d[0]);
        printf("������������0x%2X\t", d[1]);
        printf("\n");
    }
    break;
    case midi_reader::instrument:
    {
        printf("�趨����\t");
        printf("�������룺%d\t", d[0]);
        printf("\n");
    }
    break;
    case midi_reader::pressure:
    {
        printf("ͨ������ѹ��\t");
        printf("������0x%2X\t", d[0]);
        printf("\n");
    }
    break;
    case midi_reader::slip:
    {
        printf("����\t");
        printf("\n");
    }
    break;
    case midi_reader::system:
    {
        printf("ϵͳ��Ϣ��");
        switch (message_ex)
        {
        case 0x00://FF 00 ���ù������
            printf("���ù������\t");
            break;
        case 0x01://FF 01 �ı���Ϣ
            printf("�ı���Ϣ:%s\t", data_str().c_str());
            break;
        case 0x02://FF 02 ��Ȩ��Ϣ
            printf("��Ȩ��Ϣ:%s\t", data_str().c_str());
            break;
        case 0x03://FF 03 �������ƻ��������
            printf("����:%s\t", data_str().c_str());
            break;
        case 0x04://FF 04 ����
            printf("����\t", data_str().c_str());
            break;
        case 0x05://FF 05 ���
            printf("���\t", data_str().c_str());
            break;
        case 0x06://FF 06 ���
            printf("���\t", data_str().c_str());
            break;
        case 0x07://FF 07 ��ʾ��/ע��
            printf("��ʾ��/ע��\t", data_str().c_str());
            break;
        case 0x20://FF 20 MIDIͨ��
            printf("MIDIͨ��:%d\t",d[0]);
            break;
        case 0x21://FF 21 MIDI�˿�
            printf("MIDI�˿�:%d\t", d[0]);
            break;
        case 0x2F://FF 2F MTrk�������
            printf("MTrk�������\t");
            break;
        case 0x51://FF 51 �ٶ�
            printf("�����ķ���������%d us\t", sys_get_time());
            break;
        case 0x54://FF 54 SMPTEƫ��     FF 54 05 hr mn se fr ff
            printf("SMPTEƫ��\t");
            break;
        case 0x58://FF 58 ʱ��ǩ��      FF 58 04 nn dd cc bb
            printf("������Ϣ\t");
            break;
        case 0x59://FF 59 �׺���Ϣ      FF 59 02 sf mi
        {
            printf("�׺���Ϣ:");
            if (d[0] & 0x80)
                printf("��%d",0x10 -(d[0] & 0x0f));
            else
                printf("��%d",(d[0] & 0x0f));

            if (d[1])
                printf("С��");
            else
                printf("���");
        }
            break;
        case 0x7F://FF 7F ר��/��������
            printf("ר��/��������\t");
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
    int SAMPLE_NUM = sampleRate * note->time / 1000; //�ɼ���������
    int AUDIO_CYCLE = sampleRate / note->frequency; //һ�����Ҳ��ɼ���������
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

        s[i] = s[i] + (int16_t)(((float)press * sin(2 * PI * i / AUDIO_CYCLE) * 127));//ʹ��sin��������ʼ����0
    }
    return SAMPLE_NUM;
}

midi_wav_maker::midi_wav_maker()
{
    head =
    {
        {'R','I','F','F'},
        0, //�ļ�����=���ֶ�+ 8
        {'W','A','V','E'},
        {'f','m','t',' '},
        16, //��ʽ���ȣ��ڴ�֮ǰ��Ա�Ĵ�С
        1,   //�����ʽΪPCM
        1, //����
        sampleRate, //����Ƶ��
        sampleRate * 2,// ���ݴ�������
        2, //���ݿ���뵥λ(һ��sample��ռ�ֽ���)
        16,//����λ��(����)
        {'d','a','t','a'},
        0
    };//�������ݵĴ�С

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
    long SAMPLE_NUM = sampleRate* reader->midi.notes.back().end / 1000 ; //�ɼ���������
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
    ocout.open(PATH, std::ios::out | std::ios::binary);//�򿪣�������ʱ���ɣ�
    ocout.write((char*)&head, sizeof head);//���ļ�ͷ����д���ļ�
    ocout.write((char*)body.get(), SAMPLE_NUM * 2);//�������ļ�д�����
    ocout.close();//�ر��ļ�
}

