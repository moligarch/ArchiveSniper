#include "pch.h"
#include "CppUnitTest.h"
#include "../ArchiveSniper/ArchiveSniper.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(Format)
	{
	public:
		TEST_METHOD(Doc)
		{
			const std::string path{ "..\\..\\UnitTest\\sample\\sample.doc" };
			ArcSnp reader(path, false);
			auto result = reader.GetMetadata(path);
			Assert::IsTrue(result.msFilesCount > 1);
		}
		TEST_METHOD(Docx)
		{
			const std::string path{ "..\\..\\UnitTest\\sample\\sample.docx" };
			ArcSnp reader(path, false);
			auto result = reader.GetMetadata(path);
			Assert::IsTrue(result.msFilesCount  > 1);
		}
		TEST_METHOD(Msi)
		{
			const std::string path{ "..\\..\\UnitTest\\sample\\sample.msi" };
			ArcSnp reader(path, false);
			auto result = reader.GetMetadata(path);
			Assert::IsTrue(result.msFilesCount > 1);
		}
		TEST_METHOD(Rar)
		{
			const std::string path{ "..\\..\\UnitTest\\sample\\sample.rar" };
			ArcSnp reader(path, false);
			auto result = reader.GetMetadata(path);
			Assert::IsTrue(result.msFilesCount > 1);
		}
		TEST_METHOD(Zip)
		{
			const std::string path{ "..\\..\\UnitTest\\sample\\sample.zip" };
			ArcSnp reader(path, false);
			auto result = reader.GetMetadata(path);
			Assert::IsTrue(result.msFilesCount > 1);
		}
		TEST_METHOD(SevenZ)
		{
			const std::string path{ "..\\..\\UnitTest\\sample\\sample.7z" };
			ArcSnp reader(path, false);
			auto result = reader.GetMetadata(path);
			Assert::IsTrue(result.msFilesCount > 1);
		}
		TEST_METHOD(PE)
		{
			const std::string path{ "..\\..\\UnitTest\\sample\\sample.sfx" };
			ArcSnp reader(path, false);
			auto result = reader.GetMetadata(path);
			Assert::IsTrue(result.msFilesCount  > 1);
		}
	};

	TEST_CLASS(Read) {
	public:
		TEST_METHOD(GetInsiderContent)
		{
			std::string path{ "..\\..\\UnitTest\\sample\\sample.docx" };
			ArcSnp reader(path, false);
			try
			{
				auto result = reader.GetContent(path);
				auto iter = std::find_if(result.begin(), result.end(), [](const DCOMP& item) {
					return item.msBasePath == "..\\..\\UnitTest\\sample\\sample.docx->test.txt";
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
			//Open Archive and store Content (DCOMP struct store each file inside archive whit detail)
			std::string path{ "..\\..\\UnitTest\\sample\\recursive.zip" };
			ArcSnp reader(path, true, 3);
			content_t result{};
			try
			{
				result = reader.GetContent(path);
				Assert::IsTrue(result.size() > 2);
				//Assert::
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << GetLastError();
				Assert::Fail((wchar_t*)*e.what());
			}
		}
	};

	TEST_CLASS(Edit) {
	public:
		
	};
}