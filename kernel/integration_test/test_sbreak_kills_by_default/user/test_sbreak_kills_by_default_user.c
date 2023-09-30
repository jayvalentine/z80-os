/* Child process for test_sbreak_kills_by_default.
 *
 * Does not register a SIG_CANCEL handler
 * so receiving the CANCEL byte will kill the process.
 */

int user_main(void)
{
    while (1)
    {
    }
}
