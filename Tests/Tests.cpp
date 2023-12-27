#include "pch.h"
#include "CppUnitTest.h"
#include "../ArchiveSniper/ArchiveSniper.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(FormatMetadata)
	{
	public:
		TEST_METHOD(Doc)
		{
			const std::string path{ "..\\..\\Tests\\sample\\test.doc" };
			ArcSnp reader(path, false);
			auto result = reader.GetMetadata(path);
			Assert::IsTrue(result.msFilesCount > 1);
		}
		TEST_METHOD(Docx)
		{
			const std::string path{ "..\\..\\Tests\\sample\\test.docx" };
			ArcSnp reader(path, false);
			auto result = reader.GetMetadata(path);
			Assert::IsTrue(result.msFilesCount  > 0);
		}
		TEST_METHOD(Msi)
		{
			const std::string path{ "..\\..\\Tests\\sample\\test.msi" };
			ArcSnp reader(path, false);
			auto result = reader.GetMetadata(path);
			Assert::IsTrue(result.msFilesCount > 0);
		}
		TEST_METHOD(Rar)
		{
			const std::string path{ "..\\..\\Tests\\sample\\test.rar" };
			ArcSnp reader(path, false);
			auto result = reader.GetMetadata(path);
			Assert::IsTrue(result.msFilesCount > 0);
		}
		TEST_METHOD(Zip)
		{
			const std::string path{ "..\\..\\Tests\\sample\\test.zip" };
			ArcSnp reader(path, false);
			auto result = reader.GetMetadata(path);
			Assert::IsTrue(result.msFilesCount > 0);
		}
		TEST_METHOD(SevenZ)
		{
			const std::string path{ "..\\..\\Tests\\sample\\test.7z" };
			ArcSnp reader(path, false);
			auto result = reader.GetMetadata(path);
			Assert::IsTrue(result.msFilesCount > 0);
		}
		TEST_METHOD(PE)
		{
			const std::string path{ "..\\..\\Tests\\sample\\test.sfx" };
			ArcSnp reader(path, false);
			auto result = reader.GetMetadata(path);
			Assert::IsTrue(result.msFilesCount  > 0);
		}
	};

	TEST_CLASS(Read) {
	public:
		TEST_METHOD(GetContent)
		{
			std::string path{ "..\\..\\Tests\\sample\\test.docx" };
			ArcSnp reader(path, false);
			try
			{
				auto result = reader.GetContent(path);
				auto iter = std::find_if(result.begin(), result.end(), [](const DCOMP& item) {
					return item.msBasePath == "..\\..\\Tests\\sample\\test.docx->test.txt";
					});
				std::string buffer{};
				for (const auto& chr : iter->msBuffer) {
					buffer += chr;
				}
				Assert::IsTrue(buffer == "Find This For Test");
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << GetLastError();
				Assert::Fail((wchar_t*)*e.what());
			}
		}
		TEST_METHOD(GetContentRecursively)
		{
			//Open Archive and store Content (DCOMP struct store each file inside archive with detail)
			std::string path{ "..\\..\\Tests\\sample\\rc1.zip" };
			ArcSnp reader(path, true, 3);
			content_t result{};
			try
			{
				result = reader.GetContent(path);
				Assert::IsTrue(result.size() > 2);
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << GetLastError();
				Assert::Fail((wchar_t*)*e.what());
			}
		}
	};

	TEST_CLASS(Write) {
	public:
		//TODO
	};
}

namespace PerformanceTest
{
	TEST_CLASS(Read)
	{
	public:
		TEST_METHOD(Depth1)
		{

			std::string path{ "..\\..\\Tests\\sample\\test.zip" };
			ArcSnp reader(path, true, 2);
			content_t result{};
			try
			{
				result = reader.GetContent(path);
				auto iter = std::find_if(result.begin(), result.end(), [](const DCOMP& item) {
					return item.msBasePath ==
						"..\\..\\Tests\\sample\\test.zip->test.txt";
					});
				std::string buffer{};
				for (const auto& chr : iter->msBuffer) {
					buffer += chr;
				}
				Assert::IsTrue(buffer == "Find This For Test");
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << GetLastError();
				Assert::Fail((wchar_t*)*e.what());
			}
		}
		TEST_METHOD(Depth2)
		{
			
			std::string path{ "..\\..\\Tests\\sample\\rc1.zip" };
			ArcSnp reader(path, true, 2);
			content_t result{};
			try
			{
				result = reader.GetContent(path);
				auto iter = std::find_if(result.begin(), result.end(), [](const DCOMP& item) {
					return item.msBasePath ==
						"..\\..\\Tests\\sample\\rc1.zip->test.zip->test.txt";
					});
				std::string buffer{};
				for (const auto& chr : iter->msBuffer) {
					buffer += chr;
				}
				Assert::IsTrue(buffer == "Find This For Test");
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << GetLastError();
				Assert::Fail((wchar_t*)*e.what());
			}
		}
		TEST_METHOD(Depth3)
		{
			
			std::string path{ "..\\..\\Tests\\sample\\rc2.zip" };
			ArcSnp reader(path, true, 3);
			content_t result{};
			try
			{
				result = reader.GetContent(path);
				auto iter = std::find_if(result.begin(), result.end(), [](const DCOMP& item) {
					return item.msBasePath ==
						"..\\..\\Tests\\sample\\rc2.zip->rc1.zip->test.zip->test.txt";
					});
				std::string buffer{};
				for (const auto& chr : iter->msBuffer) {
					buffer += chr;
				}
				Assert::IsTrue(buffer == "Find This For Test");
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << GetLastError();
				Assert::Fail((wchar_t*)*e.what());
			}
		}
		TEST_METHOD(Depth4)
		{
			
			std::string path{ "..\\..\\Tests\\sample\\rc3.zip" };
			ArcSnp reader(path, true, 4);
			content_t result{};
			try
			{
				result = reader.GetContent(path);
				auto iter = std::find_if(result.begin(), result.end(), [](const DCOMP& item) {
					return item.msBasePath ==
						"..\\..\\Tests\\sample\\rc3.zip->rc2.zip->rc1.zip->test.zip->test.txt";
					});
				std::string buffer{};
				for (const auto& chr : iter->msBuffer) {
					buffer += chr;
				}
				Assert::IsTrue(buffer == "Find This For Test");
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << GetLastError();
				Assert::Fail((wchar_t*)*e.what());
			}
		}
		TEST_METHOD(Depth5)
		{
			
			std::string path{ "..\\..\\Tests\\sample\\rc4.zip" };
			ArcSnp reader(path, true, 5);
			content_t result{};
			try
			{
				result = reader.GetContent(path);
				auto iter = std::find_if(result.begin(), result.end(), [](const DCOMP& item) {
					return item.msBasePath ==
						"..\\..\\Tests\\sample\\rc4.zip->rc3.zip->rc2.zip->rc1.zip->test.zip->test.txt";
					});
				std::string buffer{};
				for (const auto& chr : iter->msBuffer) {
					buffer += chr;
				}
				Assert::IsTrue(buffer == "Find This For Test");
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << GetLastError();
				Assert::Fail((wchar_t*)*e.what());
			}
		}
		TEST_METHOD(Depth6)
		{

			std::string path{ "..\\..\\Tests\\sample\\rc5.zip" };
			ArcSnp reader(path, true, 6);
			content_t result{};
			try
			{
				result = reader.GetContent(path);
				auto iter = std::find_if(result.begin(), result.end(), [](const DCOMP& item) {
					return item.msBasePath ==
						"..\\..\\Tests\\sample\\rc5.zip->rc4.zip->rc3.zip->rc2.zip->rc1.zip->test.zip->test.txt";
					});
				std::string buffer{};
				for (const auto& chr : iter->msBuffer) {
					buffer += chr;
				}
				Assert::IsTrue(buffer == "Find This For Test");
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << GetLastError();
				Assert::Fail((wchar_t*)*e.what());
			}
		}
		TEST_METHOD(Depth7)
		{

			std::string path{ "..\\..\\Tests\\sample\\rc6.zip" };
			ArcSnp reader(path, true, 7);
			content_t result{};
			try
			{
				result = reader.GetContent(path);
				auto iter = std::find_if(result.begin(), result.end(), [](const DCOMP& item) {
					return item.msBasePath ==
						"..\\..\\Tests\\sample\\rc6.zip->rc5.zip->rc4.zip->rc3.zip->rc2.zip->rc1.zip->test.zip->test.txt";
					});
				std::string buffer{};
				for (const auto& chr : iter->msBuffer) {
					buffer += chr;
				}
				Assert::IsTrue(buffer == "Find This For Test");
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << GetLastError();
				Assert::Fail((wchar_t*)*e.what());
			}
		}
		TEST_METHOD(Depth8)
		{

			std::string path{ "..\\..\\Tests\\sample\\rc7.zip" };
			ArcSnp reader(path, true, 8);
			content_t result{};
			try
			{
				result = reader.GetContent(path);
				auto iter = std::find_if(result.begin(), result.end(), [](const DCOMP& item) {
					return item.msBasePath ==
						"..\\..\\Tests\\sample\\rc7.zip->rc6.zip->rc5.zip->rc4.zip->rc3.zip->rc2.zip->rc1.zip->test.zip->test.txt";
					});
				std::string buffer{};
				for (const auto& chr : iter->msBuffer) {
					buffer += chr;
				}
				Assert::IsTrue(buffer == "Find This For Test");
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << GetLastError();
				Assert::Fail((wchar_t*)*e.what());
			}
		}
		TEST_METHOD(Depth9)
		{

			std::string path{ "..\\..\\Tests\\sample\\rc8.zip" };
			ArcSnp reader(path, true, 9);
			content_t result{};
			try
			{
				result = reader.GetContent(path);
				auto iter = std::find_if(result.begin(), result.end(), [](const DCOMP& item) {
					return item.msBasePath ==
						"..\\..\\Tests\\sample\\rc8.zip->rc7.zip->rc6.zip->rc5.zip->rc4.zip->rc3.zip->rc2.zip->rc1.zip->test.zip->test.txt";
					});
				std::string buffer{};
				for (const auto& chr : iter->msBuffer) {
					buffer += chr;
				}
				Assert::IsTrue(buffer == "Find This For Test");
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << GetLastError();
				Assert::Fail((wchar_t*)*e.what());
			}
		}
		TEST_METHOD(DepthFinal)
		{

			std::string path{ "..\\..\\Tests\\sample\\rc20003.zip" };
			ArcSnp reader(path, true, 20005);
			content_t result{};
			try
			{
				result = reader.GetContent(path);
				Assert::IsTrue(result.size()>0);
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << GetLastError();
				Assert::Fail((wchar_t*)*e.what());
			}
		}
	};
}