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
    reader.print();
}

int main()
{
    test_read();
    return 0;
}