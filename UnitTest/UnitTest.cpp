#include "pch.h"
#include "CppUnitTest.h"
#include "../ArchiveSniper/ArchiveSniper.h"
#include <bit7z/bittypes.hpp>
#include <bit7z/bitmemextractor.hpp>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(Format)
	{
	public:
		TEST_METHOD(Doc)
		{
			const std::string path{ "..\\..\\UnitTest\\sample\\sample.doc" };
			auto result = ArcSnp::GetMetadata(path, "log\\docLog.txt");
			Assert::IsTrue(result.filesCount> 0);
		}
		TEST_METHOD(Docx)
		{
			const std::string path{ "..\\..\\UnitTest\\sample\\sample.docx" };
			auto result = ArcSnp::GetMetadata(path, "log\\docxLog.txt");
			Assert::IsTrue(result.filesCount > 0);
		}
		TEST_METHOD(Msi)
		{
			const std::string path{ "..\\..\\UnitTest\\sample\\sample.msi" };
			auto result = ArcSnp::GetMetadata(path, "log\\msiLog.txt");
			Assert::IsTrue(result.filesCount> 0);
		}
		TEST_METHOD(Rar)
		{
			const std::string path{ "..\\..\\UnitTest\\sample\\sample.rar" };
			auto result = ArcSnp::GetMetadata(path, "log\\rarLog.txt");
			Assert::IsTrue(result.filesCount> 0);
		}
		TEST_METHOD(Zip)
		{
			const std::string path{ "..\\..\\UnitTest\\sample\\sample.zip" };
			auto result = ArcSnp::GetMetadata(path, "log\\zipLog.txt");
			Assert::IsTrue(result.filesCount> 0);
		}
		TEST_METHOD(SevenZ)
		{
			const std::string path{ "..\\..\\UnitTest\\sample\\sample.7z" };
			auto result = ArcSnp::GetMetadata(path, "log\\7zLog.txt");
			Assert::IsTrue(result.filesCount> 0);
		}
		TEST_METHOD(PE)
		{
			const std::string path{ "..\\..\\UnitTest\\sample\\sample.sfx" };
			auto result = ArcSnp::GetMetadata(path, "log\\sfxLog.txt");
			Assert::IsTrue(result.filesCount > 0);
		}
	};

	TEST_CLASS(ReadAndWrite) {
	public:

		TEST_METHOD(ReadContentRecursively)
		{
			const std::string path{ "..\\..\\UnitTest\\sample\\sample.zip" };
			bit7z::Bit7zLibrary lib{ "7z.dll" };
			
			//Parent Archive
			bit7z::BitArchiveReader arc{ lib, path, bit7z::BitFormat::Auto };
			Assert::IsTrue(arc.itemsCount() == 4);
			
			//Extract 'inside.zip' to buffer
			bit7z::buffer_t buffer;
			auto childZip = arc.find("inside.zip");
			arc.extractTo(buffer, childZip->index());
			bit7z::BitArchiveReader inside{ lib, buffer, bit7z::BitFormat::Auto };
			Assert::IsTrue(inside.itemsCount() == 3);

			//Extract 'sample.docx' from 'inside.zip' to buffer1
			bit7z::buffer_t buffer1;
			auto childDocx = inside.find("sample.docx");
			inside.extractTo(buffer1, childDocx->index());
			bit7z::BitArchiveReader inside1{ lib, buffer1, bit7z::BitFormat::Auto };
			Assert::IsTrue(inside1.contains("poc.txt"));
			
			auto poc = inside1.find("poc.txt");
			bit7z::buffer_t buffer2;
			inside1.extractTo(buffer2, poc->index());
			std::ofstream log("log\\insiderLog.txt");
			log << "Below text inside qoute is what found inside poc.txt:\n\"";
			for (const auto& chr : buffer2) {
				log << chr;
			}
			log << "\"";
		}
	};
}