#!/bin/sh
path=`pwd`
pkg_dir=$path/pkg/
install_dir=$path/lib/install
liblua_path=$pkg_dir/lua-5.1.4.tar.gz

if [ ! -f $liblua_path ]; then
    echo "lua not exits!"
    exit 1
fi

tar xvf $liblua_path -C $pkg_dir
mv $pkg_dir/lua-5.1.4 $pkg_dir/lua

echo "install lua"
cd $pkg_dir/lua
make linux
mkdir -p $install_dir/lua/lib/
mkdir -p $install_dir/lua/include/
cp $pkg_dir/lua/src/liblua.a $install_dir/lua/lib/
cp $pkg_dir/lua/src/*.h $install_dir/lua/include/
echo "install lua done"
