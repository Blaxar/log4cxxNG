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
#include <log4cxxNG/rolling/fixedwindowrollingpolicy.h>
#include <log4cxxNG/helpers/pool.h>
#include <log4cxxNG/helpers/integer.h>
#include <log4cxxNG/helpers/stringhelper.h>
#include <log4cxxNG/helpers/optionconverter.h>
#include <log4cxxNG/helpers/loglog.h>
#include <log4cxxNG/helpers/exception.h>
#include <log4cxxNG/rolling/rolloverdescription.h>
#include <log4cxxNG/rolling/filerenameaction.h>
#include <log4cxxNG/rolling/gzcompressaction.h>
#include <log4cxxNG/rolling/zipcompressaction.h>
#include <log4cxxNG/pattern/integerpatternconverter.h>

using namespace log4cxxng;
using namespace log4cxxng::rolling;
using namespace log4cxxng::helpers;
using namespace log4cxxng::pattern;

IMPLEMENT_LOG4CXXNG_OBJECT(FixedWindowRollingPolicy)

FixedWindowRollingPolicy::FixedWindowRollingPolicy() :
	minIndex(1), maxIndex(7)
{
}

void FixedWindowRollingPolicy::setMaxIndex(int maxIndex1)
{
	this->maxIndex = maxIndex1;
}

void FixedWindowRollingPolicy::setMinIndex(int minIndex1)
{
	this->minIndex = minIndex1;
}



void FixedWindowRollingPolicy::setOption(const LogString& option,
	const LogString& value)
{
	if (StringHelper::equalsIgnoreCase(option,
			LOG4CXXNG_STR("MININDEX"),
			LOG4CXXNG_STR("minindex")))
	{
		minIndex = OptionConverter::toInt(value, 1);
	}
	else if (StringHelper::equalsIgnoreCase(option,
			LOG4CXXNG_STR("MAXINDEX"),
			LOG4CXXNG_STR("maxindex")))
	{
		maxIndex = OptionConverter::toInt(value, 7);
	}
	else
	{
		RollingPolicyBase::setOption(option, value);
	}
}

/**
 * {@inheritDoc}
 */
void FixedWindowRollingPolicy::activateOptions(Pool& p)
{
	RollingPolicyBase::activateOptions(p);

	if (maxIndex < minIndex)
	{
		LogLog::warn(
			LOG4CXXNG_STR("MaxIndex  cannot be smaller than MinIndex."));
		maxIndex = minIndex;
	}

	if ((maxIndex - minIndex) > MAX_WINDOW_SIZE)
	{
		LogLog::warn(LOG4CXXNG_STR("Large window sizes are not allowed."));
		maxIndex = minIndex + MAX_WINDOW_SIZE;
	}

	PatternConverterPtr itc = getIntegerPatternConverter();

	if (itc == NULL)
	{
		throw IllegalStateException();
	}
}

/**
 * {@inheritDoc}
 */
RolloverDescriptionPtr FixedWindowRollingPolicy::initialize(
	const   LogString&  currentActiveFile,
	const   bool        append,
	Pool&       pool)
{
	LogString newActiveFile(currentActiveFile);
	explicitActiveFile = false;

	if (currentActiveFile.length() > 0)
	{
		explicitActiveFile = true;
		newActiveFile = currentActiveFile;
	}

	if (!explicitActiveFile)
	{
		LogString buf;
		ObjectPtr obj(new Integer(minIndex));
		formatFileName(obj, buf, pool);
		newActiveFile = buf;
	}

	ActionPtr noAction;

	return new RolloverDescription(newActiveFile, append, noAction, noAction);
}

/**
 * {@inheritDoc}
 */
RolloverDescriptionPtr FixedWindowRollingPolicy::rollover(
	const   LogString&  currentActiveFile,
	const   bool        append,
	Pool&       pool)
{
	RolloverDescriptionPtr desc;

	if (maxIndex < 0)
	{
		return desc;
	}

	int purgeStart = minIndex;

	if (!explicitActiveFile)
	{
		purgeStart++;
	}

	if (!purge(purgeStart, maxIndex, pool))
	{
		return desc;
	}

	LogString buf;
	ObjectPtr obj(new Integer(purgeStart));
	formatFileName(obj, buf, pool);

	LogString renameTo(buf);
	LogString compressedName(renameTo);
	ActionPtr compressAction ;

	if (StringHelper::endsWith(renameTo, LOG4CXXNG_STR(".gz")))
	{
		renameTo.resize(renameTo.size() - 3);
		compressAction =
			new GZCompressAction(
			File().setPath(renameTo),
			File().setPath(compressedName),
			true);
	}
	else if (StringHelper::endsWith(renameTo, LOG4CXXNG_STR(".zip")))
	{
		renameTo.resize(renameTo.size() - 4);
		compressAction =
			new ZipCompressAction(
			File().setPath(renameTo),
			File().setPath(compressedName),
			true);
	}

	FileRenameActionPtr renameAction =
		new FileRenameAction(
		File().setPath(currentActiveFile),
		File().setPath(renameTo),
		false);

	desc = new RolloverDescription(
		currentActiveFile,  append,
		renameAction,       compressAction);

	return desc;
}

/**
 * Get index of oldest log file to be retained.
 * @return index of oldest log file.
 */
int FixedWindowRollingPolicy::getMaxIndex() const
{
	return maxIndex;
}

/**
 * Get index of most recent log file.
 * @return index of oldest log file.
 */
int FixedWindowRollingPolicy::getMinIndex() const
{
	return minIndex;
}


/**
 * Purge and rename old log files in preparation for rollover
 * @param lowIndex low index
 * @param highIndex high index.  Log file associated with high
 * index will be deleted if needed.
 * @return true if purge was successful and rollover should be attempted.
 */
bool FixedWindowRollingPolicy::purge(int lowIndex, int highIndex, Pool& p) const
{
	int suffixLength = 0;

	std::vector<FileRenameActionPtr> renames;
	LogString buf;
	ObjectPtr obj = new Integer(lowIndex);
	formatFileName(obj, buf, p);

	LogString lowFilename(buf);

	if (lowFilename.compare(lowFilename.length() - 3, 3, LOG4CXXNG_STR(".gz")) == 0)
	{
		suffixLength = 3;
	}
	else if (lowFilename.compare(lowFilename.length() - 4, 4, LOG4CXXNG_STR(".zip")) == 0)
	{
		suffixLength = 4;
	}

	for (int i = lowIndex; i <= highIndex; i++)
	{
		File toRenameCompressed;
		toRenameCompressed.setPath(lowFilename);
		File toRenameBase;
		toRenameBase.setPath(lowFilename.substr(0, lowFilename.length() - suffixLength));
		File* toRename = &toRenameCompressed;
		bool isBase = false;
		bool exists = toRenameCompressed.exists(p);

		if (suffixLength > 0)
		{
			if (exists)
			{
				if (toRenameBase.exists(p))
				{
					toRenameBase.deleteFile(p);
				}
			}
			else
			{
				toRename = &toRenameBase;
				exists = toRenameBase.exists(p);
				isBase = true;
			}
		}

		if (exists)
		{
			//
			//    if at upper index then
			//        attempt to delete last file
			//        if that fails then abandon purge
			if (i == highIndex)
			{
				if (!toRename->deleteFile(p))
				{
					return false;
				}

				break;
			}

			//
			//   if intermediate index
			//     add a rename action to the list
			buf.erase(buf.begin(), buf.end());
			obj = new Integer(i + 1);
			formatFileName(obj, buf, p);

			LogString highFilename(buf);
			LogString renameTo(highFilename);

			if (isBase)
			{
				renameTo =
					highFilename.substr(0, highFilename.length() - suffixLength);
			}

			renames.push_back(new FileRenameAction(*toRename, File().setPath(renameTo), true));
			lowFilename = highFilename;
		}
		else
		{
			break;
		}
	}

	//
	//   work renames backwards
	//
	for (std::vector<FileRenameActionPtr>::reverse_iterator iter = renames.rbegin();
		iter != renames.rend();
		iter++)
	{

		try
		{
			if (!(*iter)->execute(p))
			{
				return false;
			}
		}
		catch (std::exception&)
		{
			LogLog::warn(LOG4CXXNG_STR("Exception during purge in RollingFileAppender"));

			return false;
		}
	}

	return true;
}

#define RULES_PUT(spec, cls) \
	specs.insert(PatternMap::value_type(LogString(LOG4CXXNG_STR(spec)), (PatternConstructor) cls ::newInstance))


log4cxxng::pattern::PatternMap FixedWindowRollingPolicy::getFormatSpecifiers() const
{
	PatternMap specs;
	RULES_PUT("i", IntegerPatternConverter);
	RULES_PUT("index", IntegerPatternConverter);
	return specs;
}
