
#include"base/test/test_suite.h"

#include"sandbox/tests/test_utils.h"
#include"sandbox/tests/unit_test.h"

#include"testing/gtest/include/gtest/gtest.h"
#include"testing/multiprocess_func_list.h"

void RunPostTestsChecks(const base::FilePath& orig_cwd)
{
	if (TestUtils::CurrentProcessHasChildren())
	{
		LOG(FATAL) << "One of the tests created a child that was not waited for. "
			<< "Please, clean up after your tests!";
	}

	base::FilePath cwd;
	CHECK(GetCurrentDirectory(&cwd));

}

int main(int argc, char* argv[])
{

	base::FilePath orig_cwd;

#if !defined(SANDBOX_USES_BASE_TEST_SUITE)
	int test_result = RUN_ALL_TESTS();
#else
	int tests_result = base::RunUnitTestsUsingBaseTestSuite(argc, argv);
#endif

	sandbox::RunPostTestsChecks(orig_cwd);
	return test_result;
}