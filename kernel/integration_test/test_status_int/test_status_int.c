volatile int val;

void test_func(void)
{
    val = 42;
}

int main(void)
{
    while (1)
    {
        test_func();
    }
}
