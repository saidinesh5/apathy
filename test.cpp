/******************************************************************************
 * Copyright (c) 2013 Dan Lecocq
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *****************************************************************************/


#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_FAST_COMPILE
#define CATCH_CONFIG_DISABLE_MATCHERS

#include <catch.hpp>
#include <algorithm>

/* Internal libraries */
#include "path.hpp"

using namespace apathy;

TEST_CASE("path", "Path functionality works as advertised") {
    SECTION("cwd", "And equivalent vs ==") {
        Path cwd(Path::cwd());
        Path empty("");
        REQUIRE(cwd != empty);
        REQUIRE(cwd.equivalent(empty));
        REQUIRE(empty.equivalent(cwd));
        REQUIRE(cwd.is_absolute());
        REQUIRE(!empty.is_absolute());
        REQUIRE(empty.absolute() == cwd);
        REQUIRE(Path() == "");
    }

    SECTION("operator=", "Make sure assignment works as expected") {
        Path cwd(Path::cwd());
        Path empty("");
        REQUIRE(cwd != empty);
        empty = cwd;
        REQUIRE(cwd == empty);
    }

#if defined(_WIN32)

    SECTION("Path", "Make sure constructors work properly on windows")
    {
        /*On windows, we actually even replace the separator from "/" to "\" */
        REQUIRE((Path("foo/bar") + "baz").string() == "foo\\bar\\baz");
        REQUIRE((Path("C://foo/bar") + "baz").string() == "C:\\\\foo\\bar\\baz");
    }

    SECTION("operator+=", "Make sure operator<< works correctly") {
        Path root("C:");
        root << "hello" << "how" << "are" << "you";
        REQUIRE(root.string() == "C:\\hello\\how\\are\\you");

        /* It also needs to be able to accept things like floats, ints, etc. */
        root = Path("C:");
        root << "hello" << 5 << "how" << 3.14 << "are";
        REQUIRE(root.string() == "C:\\hello\\5\\how\\3.14\\are");
    }

    SECTION("operator+", "Make sure operator+ works correctly") {
        REQUIRE((Path("foo/bar") + "baz").string() == "foo\\bar\\baz");
    }

    SECTION("trim", "Make sure trim actually strips off separators") {
        REQUIRE(Path("C:\\hello\\how\\are\\you\\\\\\\\").trim().string() == "C:\\hello\\how\\are\\you");
        REQUIRE(Path("C:\\hello\\how\\are\\\\you").trim().string() == "C:\\hello\\how\\are\\\\you");
        REQUIRE(Path("C:\\hello\\how\\are\\you\\").trim().string() == "C:\\hello\\how\\are\\you");
    }

    SECTION("directory", "Make sure we can make paths into directories") {
        REQUIRE(Path("C:\\hello\\how\\are\\you").directory().string() == "C:\\hello\\how\\are\\you\\");
        REQUIRE(Path("C:\\hello\\how\\are\\you\\").directory().string() == "C:\\hello\\how\\are\\you\\");
        REQUIRE(Path("C:\\hello\\how\\are\\you\\\\").directory().string() == "C:\\hello\\how\\are\\you\\");
    }

    SECTION("relative", "Evaluates relative urls correctly") {
        REQUIRE(Path("C:\\hello\\how\\are\\you").relative(Path("foo")).string() == "C:\\hello\\how\\are\\you\\foo");
        REQUIRE(Path("C:\\hello\\how\\are\\you").relative(Path("D:\\fine\\thank\\you")).string() == "D:\\fine\\thank\\you");
    }


    SECTION("parent", "Make sure we can find the parent directory") {
        REQUIRE(Path("C:\\hello\\how\\are\\you").parent().string() == "C:\\hello\\how\\are\\");
        REQUIRE(Path("C:\\hello\\how\\are\\you").parent().parent().string() == "C:\\hello\\how\\");
        REQUIRE(Path("C:\\").parent().string() == "C:\\");

        REQUIRE(Path("").parent() != Path::cwd().parent());
        REQUIRE(Path("").parent().equivalent(Path::cwd().parent()));

        REQUIRE(Path("foo\\bar").parent().parent() == "");
        REQUIRE(Path("foo/../bar/baz/a/../").parent() == "bar\\");
    }

    SECTION("sanitize", "Make sure we can sanitize a path") {
        REQUIRE(Path("foo///bar/a/b/../c").sanitize().string() == "foo\\bar\\a\\c");
        REQUIRE(Path("..\\foo///bar\\a/b/../c").sanitize().string() == "..\\foo\\bar\\a\\c");
        REQUIRE(Path("../../a/b////c").sanitize().string() == "..\\..\\a\\b\\c");
        REQUIRE(Path("C:/../../a/b////c").sanitize().string() == "C:\\a\\b\\c");
        REQUIRE(Path("C:/./././a/./b/../../c").sanitize().string() == "C:\\c");
        REQUIRE(Path("././a/b/c/").sanitize().string() == "a\\b\\c\\");
    }

    SECTION("equivalent", "Make sure equivalent paths work") {
        REQUIRE(Path("foo////a/b/../c/").equivalent(Path("foo/a/c/")));
        REQUIRE(Path("FOO////a/b/../c/").equivalent(Path("foo/a/c/")));
        REQUIRE(Path("../foo/bar/").equivalent(Path::cwd().parent().append(Path("foo")).append(Path("bar")).directory()));
    }

#else

    SECTION("operator+=", "Make sure operator<< works correctly") {
        Path root("/");
        root << "hello" << "how" << "are" << "you";
        REQUIRE(root.string() == "/hello/how/are/you");

        /* It also needs to be able to accept things like floats, ints, etc. */
        root = Path("/");
        root << "hello" << 5 << "how" << 3.14 << "are";
        REQUIRE(root.string() == "/hello/5/how/3.14/are");
    }

    SECTION("operator+", "Make sure operator+ works correctly") {
        REQUIRE((Path("foo/bar") + "baz").string() == "foo/bar/baz");
    }

    SECTION("trim", "Make sure trim actually strips off separators") {
        REQUIRE(Path("/hello/how/are/you////").trim().string() == "/hello/how/are/you");
        REQUIRE(Path("/hello/how/are/you").trim().string() == "/hello/how/are/you");
        REQUIRE(Path("/hello/how/are/you/").trim().string() == "/hello/how/are/you");
    }

    SECTION("directory", "Make sure we can make paths into directories") {
        REQUIRE(Path("/hello/how/are/you").directory().string() == "/hello/how/are/you/");
        REQUIRE(Path("/hello/how/are/you/").directory().string() == "/hello/how/are/you/");
        REQUIRE(Path("/hello/how/are/you//").directory().string() == "/hello/how/are/you/");
    }

    SECTION("relative", "Evaluates relative urls correctly") {
        REQUIRE(Path("/hello/how/are/you").relative(Path("foo")).string() == "/hello/how/are/you/foo");
        REQUIRE(Path("/hello/how/are/you/").relative(Path("foo")).string() == "/hello/how/are/you/foo");
        REQUIRE(Path("/hello/how/are/you").relative(Path("/fine/thank/you")).string() == "/fine/thank/you");
    }

    SECTION("parent", "Make sure we can find the parent directory") {
        REQUIRE(Path("/hello/how/are/you").parent().string() == "/hello/how/are/");
        REQUIRE(Path("/hello/how/are/you").parent().parent().string() == "/hello/how/");
        
        /* / is its own parent, at least according to bash:
         *
         *    cd / && cd ..
         */
        REQUIRE(Path("/").parent().string() == "/");

        REQUIRE(Path("").parent() != Path::cwd().parent());
        REQUIRE(Path("").parent().equivalent(Path::cwd().parent()));

        REQUIRE(Path("foo/bar").parent().parent() == "");
        REQUIRE(Path("foo/../bar/baz/a/../").parent() == "bar/");
    }

    SECTION("sanitize", "Make sure we can sanitize a path") {
        REQUIRE(Path("foo///bar/a/b/../c").sanitize() == "foo/bar/a/c");
        REQUIRE(Path("../foo///bar/a/b/../c").sanitize() == "../foo/bar/a/c");
        REQUIRE(Path("../../a/b////c").sanitize() == "../../a/b/c");

        REQUIRE(Path("/../../a/b////c").sanitize() == "/a/b/c");
        REQUIRE(Path("/./././a/./b/../../c").sanitize() == "/c");

        REQUIRE(Path("././a/b/c/").sanitize() == "a/b/c/");
    }

    SECTION("equivalent", "Make sure equivalent paths work") {
        REQUIRE(Path("foo////a/b/../c/").equivalent(Path("foo/a/c/")));
        REQUIRE(Path("../foo/bar/").equivalent(Path::cwd().parent().append("foo").append("bar").directory()));
    }

#endif

    SECTION("makedirs", "Make sure we recursively make directories") {
        Path path("foo");
        REQUIRE(!path.exists());
        path << "bar" << "baz" << "whiz";
        Path::makedirs(path);
        REQUIRE(path.exists());
        REQUIRE(path.is_directory());

        /* Now, we should remove the directories, make sure it's gone. */
        REQUIRE(Path::rmdirs("foo"));
        REQUIRE(!Path("foo").exists());
    }

    SECTION("listdirs", "Make sure we can list directories") {
        Path path("foo");
        path << "bar" << "baz" << "whiz";
        Path::makedirs(path);
        REQUIRE(path.exists());

        /* Now touch some files in this area */
        Path::touch(Path(path).append("a"));
        Path::touch(Path(path).append("b"));
        Path::touch(Path(path).append("c"));

        /* Now list that directory */
        std::vector<Path> files = Path::listdir(path);
        REQUIRE(files.size() == 3);
        /* listdir doesn't enforce any ordering */
        REQUIRE((std::find(files.begin(), files.end(), 
            Path(path).absolute().append("a").string()) != files.end()));
        REQUIRE((std::find(files.begin(), files.end(), 
            Path(path).absolute().append("b").string()) != files.end()));
        REQUIRE((std::find(files.begin(), files.end(), 
            Path(path).absolute().append("c").string()) != files.end()));

        REQUIRE(Path::rmdirs("foo"));
        REQUIRE(!Path("foo").exists());
    }

    SECTION("rm", "Make sure we can remove files we create") {
        REQUIRE(!Path("foo").exists());
        Path::touch("foo");
        REQUIRE( Path("foo").exists());
        Path::rm("foo");
        REQUIRE(!Path("foo").exists());
    }

    SECTION("move", "Make sure we can move files / directories") {
        /* We should be able to move it in the most basic case */
        Path source("foo");
        Path dest("bar");
        REQUIRE(!source.exists());
        REQUIRE(!  dest.exists());
        Path::touch(source);

        REQUIRE(Path::move(source, dest));
        REQUIRE(!source.exists());
        REQUIRE(   dest.exists());

        REQUIRE(Path::rm(dest));
        REQUIRE(!source.exists());
        REQUIRE(!  dest.exists());

        /* And now, when the directory doesn't exist */
        dest = "bar/baz";
        REQUIRE(!dest.parent().exists());
        Path::touch(source);

        REQUIRE(!Path::move(source, dest));
        REQUIRE( Path::move(source, dest, true));
        REQUIRE(!source.exists());
        REQUIRE(   dest.exists());
        Path::rmdirs("bar");
        REQUIRE(!Path("bar").exists());
    }

    SECTION("split", "Make sure we can get segments out") {
        Path a("foo/bar/baz");
        std::vector<Path::Segment> segments(a.split());
        REQUIRE(segments.size() == 3);
        REQUIRE(segments[0].segment == "foo");
        REQUIRE(segments[1].segment == "bar");
        REQUIRE(segments[2].segment == "baz");

        a = Path("foo/bar/baz/");
        REQUIRE(a.split().size() == 4);

        a = Path("/foo/bar/baz/");
        REQUIRE(a.split().size() == 5);
    }

    SECTION("extension", "Make sure we can accurately get th file extension") {
        /* Works in a basic way */
        REQUIRE(Path("foo/bar/baz.out").extension() == "out");
        /* Gets the outermost extension */
        REQUIRE(Path("foo/bar.baz.out").extension() == "out");
        /* Doesn't take extensions from directories */
        REQUIRE(Path("foo/bar.baz/out").extension() == "");
    }

    SECTION("stem", "Make sure we can get the path stem") {
        /* Works in a basic way */
        REQUIRE(Path("foo/bar/baz.out").stem() == Path("foo/bar/baz"));
        /* Gets the outermost extension */
        REQUIRE(Path("foo/bar.baz.out").stem() == Path("foo/bar.baz"));
        /* Doesn't take extensions from directories */
        REQUIRE(Path("foo/bar.baz/out").stem() == Path("foo/bar.baz/out"));

        /* Can be used to successively pop off the extension */
        Path a("foo.bar.baz.out");
        a = a.stem(); REQUIRE(a == Path("foo.bar.baz"));
        a = a.stem(); REQUIRE(a == Path("foo.bar"));
        a = a.stem(); REQUIRE(a == Path("foo"));
        a = a.stem(); REQUIRE(a == Path("foo"));
    }

#if !defined(_WIN32)
    SECTION("glob", "Make sure glob works") {
        /* We'll touch a bunch of files to work with */
        Path::makedirs("foo");
        Path::touch("foo/bar");
        Path::touch("foo/bar2");
        Path::touch("foo/bar3");
        Path::touch("foo/baz");
        Path::touch("foo/bazzy");
        Path::touch("foo/foo");

        /* Make sure we can get it to work in a few basic ways */
        REQUIRE(Path::glob("foo/*"   ).size() == 6);
        REQUIRE(Path::glob("foo/b*"  ).size() == 5);
        REQUIRE(Path::glob("foo/baz*").size() == 2);
        REQUIRE(Path::glob("foo/ba?" ).size() == 2);

        /* Now, we should remove the directories, make sure it's gone. */
        REQUIRE(Path::rmdirs("foo"));
        REQUIRE(!Path("foo").exists());
    }
#endif

    SECTION("recursive_listdir", "Make sure we can recursively list directory") {
        /* We'll touch a bunch of files to work with */
        Path::makedirs("foo");
        Path::makedirs("foo/bar");
        Path::makedirs("foo/bar2");
        Path::makedirs("foo/bar2/bar3");
        Path::touch("foo/1");
        Path::touch("foo/2");
        Path::touch("foo/bar/1");
        Path::touch("foo/bar/2");
        Path::touch("foo/bar2/1");
        Path::touch("foo/bar2/2");

        /* Make sure we can get it to work in a few basic ways */
        REQUIRE(Path::recursive_listdir("foo").size() == 9);


        /* Now, we should remove the directories, make sure it's gone. */
        REQUIRE(Path::rmdirs(Path("foo")));
        REQUIRE(!Path("foo").exists());
    }
}
