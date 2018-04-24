
int RunUnitTestsUsingBaseTestSuite(int argc, char** argv)
{
	TestSuite test_suite(argc, argv);
	return LaunchUnitTests(argc, argv, Bind(&TestSuite::Run, Unretained(&test_suite)));
}