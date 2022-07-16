volatile int val;

void test_func()
{
    val = 42;
}

int main()
{
    while (1)
    {
        test_func();
    }
}
