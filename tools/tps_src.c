int calc_body2(int value1)
{
    return value1 = value1 + 100;
}

int calc_body(int value1, int value2, int value3, int value4)
{
    return calc_body2(value1 = value2 + value3 + value4);
}

int g_value1 = 0;
int g_value3 = 49;

int calc(void)
{
    int value2 = 2 + (3 * g_value1);
    return calc_body(g_value1 = (1 + 2) * 3, value2, g_value3, 51);
}

int/* BOOL */ DllMain(
    long long /* HINSTANCE */, unsigned long/* DWORD */, long long /* LPVOID */)
{
    return 1/* TRUE */;
}
