/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#if defined(_MSC_VER)
	#pragma warning ( disable: 4231 4251 4275 4786 )
#endif

#include <log4cxxNG/logstring.h>
#include <log4cxxNG/helpers/syslogwriter.h>
#include <log4cxxNG/helpers/loglog.h>
#include <log4cxxNG/helpers/inetaddress.h>
#include <log4cxxNG/helpers/datagramsocket.h>
#include <log4cxxNG/helpers/datagrampacket.h>
#include <log4cxxNG/helpers/transcoder.h>

using namespace log4cxxng;
using namespace log4cxxng::helpers;

SyslogWriter::SyslogWriter(const LogString& syslogHost1, int syslogHostPort1)
	: syslogHost(syslogHost1), syslogHostPort(syslogHostPort1)
{
	try
	{
		this->address = InetAddress::getByName(syslogHost1);
	}
	catch (UnknownHostException& e)
	{
		LogLog::error(((LogString) LOG4CXXNG_STR("Could not find ")) + syslogHost1 +
			LOG4CXXNG_STR(". All logging will FAIL."), e);
	}

	try
	{
		this->ds = new DatagramSocket();
	}
	catch (SocketException& e)
	{
		LogLog::error(((LogString) LOG4CXXNG_STR("Could not instantiate DatagramSocket to ")) + syslogHost1 +
			LOG4CXXNG_STR(". All logging will FAIL."), e);
	}
}

void SyslogWriter::write(const LogString& source)
{
	if (this->ds != 0 && this->address != 0)
	{
		LOG4CXXNG_ENCODE_CHAR(data, source);

		DatagramPacketPtr packet(
			new DatagramPacket((void*) data.data(), data.length(),
				address, syslogHostPort));

		ds->send(packet);
	}
}
