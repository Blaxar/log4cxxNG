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

#ifndef _LOG4CXXNG_HELPERS_READER_H
#define _LOG4CXXNG_HELPERS_READER_H

#include <log4cxxNG/helpers/objectimpl.h>

namespace log4cxxng
{

namespace helpers
{

/**
 * Abstract class for reading from character streams.
 *
 */
class LOG4CXXNG_EXPORT Reader : public ObjectImpl
{
	public:
		DECLARE_ABSTRACT_LOG4CXXNG_OBJECT(Reader)
		BEGIN_LOG4CXXNG_CAST_MAP()
		LOG4CXXNG_CAST_ENTRY(Reader)
		END_LOG4CXXNG_CAST_MAP()

	protected:
		/**
		 * Creates a new character-stream reader.
		 */
		Reader();

		virtual ~Reader();

	public:
		/**
		 * Closes the stream.
		 * @param p The memory pool associated with the reader.
		 */
		virtual void close(Pool& p) = 0;

		/**
		 * @return The complete stream contents as a LogString.
		 * @param p The memory pool associated with the reader.
		 */
		virtual LogString read(Pool& p) = 0;

	private:
		Reader(const Reader&);

		Reader& operator=(const Reader&);
};

LOG4CXXNG_PTR_DEF(Reader);
} // namespace helpers

}  //namespace log4cxxng

#endif //_LOG4CXXNG_HELPERS_READER_H
