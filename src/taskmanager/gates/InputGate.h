#pragma once

#include <atomic>

namespace lute::tm::gates {

/**
 * @class InputGate
 * 
 * @tparam RecordType Type of Record that InputGate ingests from upstream
 * 
 * @note This is supposed to be an interface, does not provide implemenation per se
 */
template<typename RecordType>
class InputGate {
public:
    /**
     * @brief Construct InputGate
     * 
     * @param capacity Buffer Size of Input Gate
     */
    InputGate(const std::size_t capacity);

    struct RecordBatch {
        RecordType* data;
        std::size_t recordCount;
    };

    /**
     * @brief Returns pointer to Record buffer for reading through argument and the number of records to read
     * 
     * @param maxRecords Maximum number of records requested; default to 1.
     * 
     * @return \c RecordBatch containing pointer and number of records to read
     * 
     * @thread Operator Thread runs fetch
     * 
     * @see commit
     */
    RecordBatch fetch(const std::size_t maxRecords = 1U) noexcept;


    /**
     * @brief Intimates safe ingestion of records by the operator. The architecture expects the operator to call it after it has successfully 
     * read input record and written output record safely. This defines the buffer positions that are now free to be overwritten.
     * 
     * @note The architecture expects \ref commit to be a very lightweight method, if it does any work at all. The cleanup logic can live in the
     * InputGate's protected methods. This is an aspect that determines how lightweight the InputGate is for the Operator thread to use.
     * 
     * @thread Operator Thread runs commit
     * 
     * @see fetch
     * @see commit_
     */
    void commit(const std::size_t commitSize = 1U) noexcept;


protected:

    /**
     * @brief Implements the work load involved in cleanup of the buffer. Is supposed to be either called by \ref commit, 
     * wait for wake up to do the clean up, or never be called at all (in case the load of cleanup is taken by the operator)
     * 
     * @note The architecture expects \ref commit to be a very lightweight method, if it does any work at all. The cleanup logic can live in the
     * InputGate's protected methods. This is an aspect that determines how lightweight the InputGate is for the Operator thread to use.
     * 
     * @thread InputGate Thread runs commit
     */
    void commit_(const std::size_t commitSize = 1U) noexcept;

private:
    RecordType* buffer_;
    size_t capacity_;

    alignas(64) std::atomic<std::size_t> writeIdx_;
    alignas(64) std::atomic<std::size_t> readIdx_;
};

} // lute::tm::gates
