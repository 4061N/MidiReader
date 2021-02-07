#include <fstream>
#include <iostream>
#include <assert.h>
#include "MidiReader.h"

using namespace std;

void test_read()
{
    const char * file_name = "test.mid";
    MidiReader reader;
    reader.read(file_name);
    //reader.print();//ȫ�����
    reader.midi.print_notes();//�����������
}

void test_frequency()
{
    midi_note midi;
    midi.set_frequency(48);
    cout << midi.frequency << endl;//�������C��Ƶ��
}

int main()
{
    test_read();
    return 0;
}