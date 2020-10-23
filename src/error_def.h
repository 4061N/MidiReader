#ifndef ERROR_DEF_h
#define ERROR_DEF_h

#define PRINT_ERROR(error_code) std::cout << get_error_msg(error_code) << std::endl;

enum ERROR_CODE
{
    // �ɹ�
    OK = 1,
    // ��ȡ�ļ�����
    ERROR_READ_FILE = -1,
    // д���ļ�����
    ERROR_WRITE_FILE = -2,
    // ��ȡλ�ó����ļ���С
    ERROR_OVER_RANGE = -3,
    // ��ȡ�ļ�ͷ����
    ERROR_READ_HEADER = -4,
};


const char* get_error_msg(ERROR_CODE error_code)
{
    switch (error_code)
    {
    case OK:
        return "operation success";
        break;
    case ERROR_READ_FILE:
        return "read failed";
        break;
    case ERROR_WRITE_FILE:
        return "write failed";
        break;
    case ERROR_OVER_RANGE:
        return "over range to read file";
        break;
    case ERROR_READ_HEADER:
        return "read header failed";
        break;
    default:
        return "";
        break;
    }
}

#endif