#include <fstream>
#include <iostream>
#include <assert.h>
#include "MidiReader.h"

using namespace std;

void test_fstream()
{
    const string file_name = "D:\\MidiReader\\src\\test.txt";
    fstream fs(file_name);
    assert(fs.is_open());
    char first[4] = {};
    fs.read(first, 4);
    cout << first;
    system("pause");
}

void test_read()
{
    const char * file_name = "test.mid";
    MidiReader reader;
    reader.read(file_name);
    //MidiFile file;
    //reader.read_file(file);

    reader.print_file(reader.midi);


    int i;
    for (i = 0; i < 4; i++)
    {
        printf("%X ", reader.test_s[i]);
    }
}

void test_app()
{
    const char * file_name = "test.mid";
    ifstream ifs;
    ifs.open(file_name, ios_base::in | ios_base::binary);
    
    if (!ifs.is_open())
    {
        cout << "can not open" << endl;
        return;
    }
    vector<char> vec;
    vec.resize(50000);
    char * buf = new char(50000);
    int count = 0;
    while (!ifs.eof())
    {
        ifs.read(vec.data(), 50000);
        cout << vec.data();
        cout << ifs.gcount();
        count++;
    }
    int a = 1;
}

void test_new()
{
    char * c = new char;
    for (int i = 0; i < 1000; ++i)
    {
        c[i] = 1;
    }
    int a = 10;
}

void test_head()
{
    const char* file_name = "test.mid";
    MidiReader reader;
    reader.MF_interface.read(file_name);
    reader.read_header();
    reader.print_header(reader.midi.header);
    int i;
    for (i = 0; i < 4; i++)
    {
        printf("%X ", reader.test_s[i]);
    }



    /*
    char* s[16];
    reader.MF_interface.read(file_name);
    reader.MF_interface.get(s, 16, true);
    int i;
    for (i = 0; i < 4; i++)
    {
        printf("%X ", s[i]);
    }*/
    //reader.print_file(reader.midi);
}

int main()
{
    //test_app();
    test_read();
    //test_new();
    //test_head();
    return 0;
}