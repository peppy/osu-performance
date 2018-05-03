#include <Shared.h>

#include "DatabaseConnection.h"
#include "UpdateBatch.h"

CUpdateBatch::CUpdateBatch(std::shared_ptr<CDatabaseConnection> pDB, u32 sizeThreshold)
: _pDB{std::move(pDB)}, _sizeThreshold{sizeThreshold}
{
	reset();
}

CUpdateBatch::~CUpdateBatch()
{
	// If we are not empty we want to commit what's left in here
	if (!_empty)
		execute();
}

CUpdateBatch::CUpdateBatch(CUpdateBatch&& other)
{
	*this = std::move(other);
}

CUpdateBatch& CUpdateBatch::operator=(CUpdateBatch&& other)
{
	std::lock_guard<std::mutex> otherLock{other._batchMutex};
	std::lock_guard<std::mutex> lock{_batchMutex};

	_sizeThreshold = other._sizeThreshold;
	_empty = other._empty;
	_pDB = std::move(other._pDB);
	_query = std::move(other._query);

	return *this;
}

void CUpdateBatch::AppendAndCommit(const std::string& values)
{
	std::lock_guard<std::mutex> lock{_batchMutex};
	AppendAndCommitNonThreadsafe(values);
}

void CUpdateBatch::AppendAndCommitNonThreadsafe(const std::string& values)
{
	append(values);

	if (Size() > _sizeThreshold)
	{
		execute();
		reset();
	}
}

void CUpdateBatch::reset()
{
	_query = "";//"START TRANSACTION;";
	_empty = true;
}

const std::string& CUpdateBatch::query()
{
	//m_Query += "COMMIT;";
	return _query;
}

void CUpdateBatch::execute()
{
	_pDB->NonQueryBackground(query());

	/*FILE* pFile = fopen("./updates.log", "ab");

	if(pFile != nullptr)
	{
		fputs(m_Query.c_str(), pFile);
		fclose(pFile);
	}*/
}
