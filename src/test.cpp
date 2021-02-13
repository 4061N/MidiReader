#include <fstream>
#include <iostream>
#include <assert.h>
#include "MidiReader.h"
#include "windows.h"

#define _USE_MATH_DEFINES
#include "math.h"

using namespace std;


#define sampleRate 44100


void test()
{
    
    const char* file_name = "test2.mid";
    MidiReader reader;
    reader.read(file_name);
    reader.make_wav("make.wav");

}

void test_read()
{
    const char * file_name = "test.mid";
    MidiReader reader;
    reader.read(file_name);
    reader.print();//ȫ�����
    //reader.midi.print_notes();//�����������
    //reader.midi.print_tracks();//�����������
}

void test_frequency()
{
    midi_note midi;
    midi.set_frequency(48);
    cout << midi.frequency << endl;//�������C��Ƶ��
}

int main()
{

    test();
	//test_read();

    return 0;
}

