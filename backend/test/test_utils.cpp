#include <iostream>
#include <gtest/gtest.h>

#include "Utils/String.h"
#include "Utils/Path.h"

static double requiredPrecision(1e-10);

using namespace STIMWALKER_NAMESPACE;

// Start the tests

TEST(String, conversions)
{
    utils::String str(utils::String::to_string(M_PI));
    double pi = std::stod(str);
    EXPECT_EQ(pi, M_PI);
}

TEST(Path, Create)
{
    {
        utils::Path emptyPath;
        EXPECT_STREQ(emptyPath.absolutePath().c_str(), utils::Path::currentDir().c_str());
        EXPECT_STREQ(emptyPath.absoluteFolder().c_str(), utils::Path::currentDir().c_str());
        EXPECT_STREQ(emptyPath.filename().c_str(), "");
        EXPECT_STREQ(emptyPath.extension().c_str(), "");
        EXPECT_STREQ(emptyPath.originalPath().c_str(), "");
    }

    {
        EXPECT_STREQ(
            utils::Path::toUnixFormat("MyUgly\\\\WindowsPath\\\\ToMyFile.txt").c_str(),
            utils::String ("MyUgly/WindowsPath/ToMyFile.txt").c_str());

        EXPECT_STREQ(
            utils::Path::toWindowsFormat("MyCute/UnixPath/ToMyFile.txt").c_str(),
            utils::String ("MyCute\\\\UnixPath\\\\ToMyFile.txt").c_str());
    }

    {
        utils::String myPathInUglyWindowsStyleString("MyUgly\\\\WindowsPath\\\\ToMyFile.txt");
        utils::Path myPathInUglyWindowsStyle(myPathInUglyWindowsStyleString);
        EXPECT_STREQ(myPathInUglyWindowsStyle.relativePath().c_str(), "./MyUgly/WindowsPath/ToMyFile.txt");
        EXPECT_STREQ(myPathInUglyWindowsStyle.originalPath().c_str(), myPathInUglyWindowsStyleString.c_str());
    }

    {
#ifdef _WIN32
        utils::String path(
            "C:\\MyLovely\\AbsolutePath\\ToMyLovelyFile.txt");
        utils::String unixPath(
            "C:/MyLovely/AbsolutePath/ToMyLovelyFile.txt");
        utils::String absoluteUnixFolder("C:/MyLovely/AbsolutePath/");
#else
        utils::String path(
            "/MyLovely/AbsolutePath/ToMyLovelyFile.txt");
        utils::String unixPath(
            "/MyLovely/AbsolutePath/ToMyLovelyFile.txt");
        utils::String absoluteUnixFolder("/MyLovely/AbsolutePath/");
#endif

        utils::Path absolutePath(path);
        EXPECT_STREQ(absolutePath.absolutePath().c_str(), unixPath.c_str());
        EXPECT_STREQ(absolutePath.absoluteFolder().c_str(), absoluteUnixFolder.c_str());
        EXPECT_STREQ(absolutePath.filename().c_str(), "ToMyLovelyFile");
        EXPECT_STREQ(absolutePath.extension().c_str(), "txt");
        EXPECT_STREQ(absolutePath.originalPath().c_str(), path.c_str());
    }

    {
        utils::Path relativePath("MyLovely/RelativePath/ToMyLovelyFile.txt");
        EXPECT_STREQ(
            relativePath.absolutePath().c_str(), 
            (utils::Path::currentDir() + "MyLovely/RelativePath/ToMyLovelyFile.txt").c_str());
        EXPECT_STREQ(
            relativePath.absoluteFolder().c_str(), (utils::Path::currentDir() + "MyLovely/RelativePath/").c_str());
        EXPECT_STREQ(relativePath.filename().c_str(), "ToMyLovelyFile");
        EXPECT_STREQ(relativePath.extension().c_str(), "txt");
        EXPECT_STREQ(relativePath.relativePath().c_str(), "./MyLovely/RelativePath/ToMyLovelyFile.txt");
        EXPECT_STREQ(relativePath.originalPath().c_str(), "MyLovely/RelativePath/ToMyLovelyFile.txt");
    }

    {
        utils::Path
        weirdRelativePath("./MyLovely/RelativePath/ToMyLovelyFile.txt");
        EXPECT_STREQ(
            weirdRelativePath.absolutePath().c_str(),
            (utils::Path::currentDir() + "MyLovely/RelativePath/ToMyLovelyFile.txt").c_str());
        EXPECT_STREQ(
            weirdRelativePath.absoluteFolder().c_str(), (utils::Path::currentDir() + "MyLovely/RelativePath/").c_str());
        EXPECT_STREQ(weirdRelativePath.filename().c_str(), "ToMyLovelyFile");
        EXPECT_STREQ(weirdRelativePath.extension().c_str(), "txt");
        EXPECT_STREQ(weirdRelativePath.relativePath().c_str(), "./MyLovely/RelativePath/ToMyLovelyFile.txt");
        EXPECT_STREQ(weirdRelativePath.originalPath().c_str(), "./MyLovely/RelativePath/ToMyLovelyFile.txt");
    }

    {
        utils::Path
        parentRelativePath("../MyLovely/ParentPath/ToMyLovelyFile.txt");
        EXPECT_STREQ(
            parentRelativePath.absolutePath().c_str(), 
            (utils::Path::currentDir() + "../MyLovely/ParentPath/ToMyLovelyFile.txt").c_str());
        EXPECT_STREQ(
            parentRelativePath.absoluteFolder().c_str(), 
            (utils::Path::currentDir() + "../MyLovely/ParentPath/").c_str());
        EXPECT_STREQ(parentRelativePath.filename().c_str(), "ToMyLovelyFile");
        EXPECT_STREQ(parentRelativePath.extension().c_str(), "txt");
        EXPECT_STREQ(parentRelativePath.relativePath().c_str(), "../MyLovely/ParentPath/ToMyLovelyFile.txt");
        EXPECT_STREQ(parentRelativePath.originalPath().c_str(), "../MyLovely/ParentPath/ToMyLovelyFile.txt");
    }

    {
        utils::Path noPath("MyLonelyFile.txt");
        EXPECT_STREQ(noPath.absolutePath().c_str(), (utils::Path::currentDir() + "MyLonelyFile.txt").c_str());
        EXPECT_STREQ(noPath.absoluteFolder().c_str(), utils::Path::currentDir().c_str());
        EXPECT_STREQ(noPath.filename().c_str(), "MyLonelyFile");
        EXPECT_STREQ(noPath.extension().c_str(), "txt");
        EXPECT_STREQ(noPath.relativePath().c_str(), "./MyLonelyFile.txt");
        EXPECT_STREQ(noPath.originalPath().c_str(), "MyLonelyFile.txt");
    }

    {
        utils::Path almostNoPath("./MyKinbDofLonelyFile.txt");
        EXPECT_STREQ(
            almostNoPath.absolutePath().c_str(), (utils::Path::currentDir() + "MyKinbDofLonelyFile.txt").c_str());
        EXPECT_STREQ(
            almostNoPath.absoluteFolder().c_str(), utils::Path::currentDir().c_str());
        EXPECT_STREQ(almostNoPath.filename().c_str(), "MyKinbDofLonelyFile");
        EXPECT_STREQ(almostNoPath.extension().c_str(), "txt");
        EXPECT_STREQ(almostNoPath.relativePath().c_str(), "./MyKinbDofLonelyFile.txt");
        EXPECT_STREQ(almostNoPath.originalPath().c_str(), "./MyKinbDofLonelyFile.txt");
    }
}

TEST(Path, Copy)
{
    utils::Path mainPath("MyLovelyPath.txt");
    utils::Path copy(mainPath);

    EXPECT_STREQ(mainPath.relativePath().c_str(), "./MyLovelyPath.txt");
    EXPECT_STREQ(copy.relativePath().c_str(), "./MyLovelyPath.txt");
    EXPECT_STREQ(mainPath.originalPath().c_str(), "MyLovelyPath.txt");
    EXPECT_STREQ(copy.originalPath().c_str(), "MyLovelyPath.txt");

    // Changing the ShallowCopy should change the Main, but not the Deep
    copy.setFilename("MySecondLovelyPath");
    copy.setExtension("newExt");

    EXPECT_STREQ(mainPath.relativePath().c_str(), "./MyLovelyPath.txt");
    EXPECT_STREQ(copy.relativePath().c_str(), "./MySecondLovelyPath.newExt");
    EXPECT_STREQ(mainPath.originalPath().c_str(), "MyLovelyPath.txt");
    EXPECT_STREQ(copy.originalPath().c_str(), "MyLovelyPath.txt");
}
