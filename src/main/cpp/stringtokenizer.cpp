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

#include <log4cxxNG/logstring.h>
#include <log4cxxNG/helpers/stringtokenizer.h>
#include <log4cxxNG/helpers/exception.h>
#if !defined(LOG4CXXNG)
	#define LOG4CXXNG 1
#endif
#include <log4cxxNG/private/log4cxxNG_private.h>

using namespace log4cxxng;
using namespace log4cxxng::helpers;

StringTokenizer::StringTokenizer(const LogString& str, const LogString& delim1)
	: src(str), delim(delim1), pos(0)
{
}

StringTokenizer::~StringTokenizer()
{
}

bool StringTokenizer::hasMoreTokens() const
{
	return (pos != LogString::npos
			&& src.find_first_not_of(delim, pos) != LogString::npos);
}

LogString StringTokenizer::nextToken()
{
	if (pos != LogString::npos)
	{
		size_t nextPos = src.find_first_not_of(delim, pos);

		if (nextPos != LogString::npos)
		{
			pos = src.find_first_of(delim, nextPos);

			if (pos == LogString::npos)
			{
				return src.substr(nextPos);
			}

			return src.substr(nextPos, pos - nextPos);
		}
	}

	throw NoSuchElementException();
#if LOG4CXXNG_RETURN_AFTER_THROW
	return LogString();
#endif
}
