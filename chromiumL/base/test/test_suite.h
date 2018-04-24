#ifndef BASE_TEST_SUITE_H
#define BASE_TEST_SUITE_H

int RunUnitTestUsingBaseTestSuite(int argc, char** argv);

class TestSuite {
public:

protected:

private:

	test::TraceToFile trace_to_file_;
	bool initialized_command_line_;
	test::ScopedFeatureList scoped_feature_list_;

};

#endif
