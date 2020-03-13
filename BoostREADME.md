If you have troubles installing Boost, here is what helped in my case:

In case if you already tried to install Boost, but it didn't work:

cd /usr/local/include
sudo rm -rf boost
cd ../lib
sudo rm -rf libboost*

This is taken from https://stackoverflow.com/questions/12578499/how-to-install-boost-on-ubuntu. Change the Boost version to 1.68 or later.

Get the version of Boost that you require. This is for 1.68 but feel free to change or manually download yourself:

wget -O boost_1_68_0.tar.gz https://sourceforge.net/projects/boost/files/boost/1.68.0/boost_1_68_0.tar.gz/download
tar xzvf boost_1_68_0.tar.gz
cd boost_1_68_0/

Get the required libraries, main ones are icu for boost::regex support:

    sudo apt-get update
    sudo apt-get install build-essential g++ python-dev autotools-dev libicu-dev build-essential libbz2-dev libboost-all-dev

Boost's bootstrap setup:

./bootstrap.sh --prefix=/usr/local

Then build it with:

./b2

and eventually install it:

sudo ./b2 install

