# Copyright 2007-2015 QReal Research Group
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

include(global.pri)

TEMPLATE = app

TARGET = testCoverage

CONFIG += c++11

SOURCES += \
	$$PWD/qrtest/unitTests/testCoverage/main.cpp \
	$$PWD/qrtest/unitTests/testCoverage/dirtree.cpp

HEADERS += \
	$$PWD/qrtest/unitTests/testCoverage/dirtree.h

OTHER_FILES += \
	$$PWD/qrtest/unitTests/testCoverage/~testignore \
	$$PWD/qrtest/unitTests/testCoverage/testpathes

copyToDestdir($$PWD/qrtest/unitTests/testCoverage/~testignore, now)
copyToDestdir($$PWD/qrtest/unitTests/testCoverage/testpathes, now)