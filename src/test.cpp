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
    //reader.print();//全部输出
    reader.midi.print_notes();//输出音符数据
}

void test_frequency()
{
    midi_note midi;
    midi.set_frequency(48);
    cout << midi.frequency << endl;//输出中央C的频率
}

int main()
{
    test_read();
    return 0;
}