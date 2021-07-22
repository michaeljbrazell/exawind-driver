#ifndef EXAWINDSOLVER_H
#define EXAWINDSOLVER_H

#include "Timers.h"
#include "ParallelPrinter.h"
#include <sys/resource.h>
#include <fstream>

namespace exawind {

class ExawindSolver
{
public:
    ExawindSolver() : m_timers(m_names){};

    void call_init_prolog(bool multi_solver_mode = true)
    {
        init_prolog(multi_solver_mode);
    };
    void call_init_epilog() { init_epilog(); };
    void call_prepare_solver_prolog() { prepare_solver_prolog(); };
    void call_prepare_solver_epilog() { prepare_solver_epilog(); };
    void call_pre_advance_stage1()
    {
        const std::string name = "Pre";
        m_timers.tick(name);
        pre_advance_stage1();
        m_timers.tock(name);
    };
    void call_pre_advance_stage2()
    {
        const std::string name = "Pre";
        m_timers.tick(name, true);
        pre_advance_stage2();
        m_timers.tock(name);
    };
    void call_advance_timestep()
    {
        const std::string name = "Solve";
        m_timers.tick(name);
        advance_timestep();
        m_timers.tock(name);
    };
    void call_post_advance()
    {
        const std::string name = "Post";
        m_timers.tick(name);
        post_advance();
        m_timers.tock(name);
    };
    void call_pre_overset_conn_work()
    {
        const std::string name = "PreConn";
        m_timers.tick(name);
        pre_overset_conn_work();
        m_timers.tock(name);
    };
    void call_post_overset_conn_work()
    {
        const std::string name = "PostConn";
        m_timers.tick(name);
        post_overset_conn_work();
        m_timers.tock(name);
    };
    void call_register_solution()
    {
        const std::string name = "Register";
        m_timers.tick(name);
        register_solution();
        m_timers.tock(name);
    };
    void call_update_solution()
    {
        const std::string name = "Update";
        m_timers.tick(name);
        update_solution();
        m_timers.tock(name);
    };
    void echo_timers(const int step)
    {
        int rank, minrank;
        MPI_Comm_rank(comm(), &rank);
        MPI_Allreduce(&rank, &minrank, 1, MPI_INT, MPI_MIN, comm());
        ParallelPrinter printer(comm(), minrank);
        const auto timings = m_timers.get_timings(comm(), printer.io_rank());
        const std::string out =
            identifier() + " WCTime at step: " + std::to_string(step);
        printer.echo(out + " " + timings);
    }

    long mem_usage()
    {
        struct rusage usage;
        getrusage(RUSAGE_SELF, &usage);
        
        // convert to MB
        const long mem = (long) ((double) usage.ru_maxrss)/1024.0;

        int myrank, psize, minrank;
        MPI_Comm_rank(comm(), &myrank);
        MPI_Comm_size(comm(), &psize);

        MPI_Allreduce(&myrank, &minrank, 1, MPI_INT, MPI_MIN, comm());
        
        // gather all memory usage to proc 0
        long minmem = -1;
        long totmem = -1;
        long maxmem = -1;
        MPI_Reduce(&mem, &minmem, 1, MPI_LONG, MPI_MIN, minrank, comm());
        MPI_Reduce(&mem, &totmem, 1, MPI_LONG, MPI_SUM, minrank, comm());
        MPI_Reduce(&mem, &maxmem, 1, MPI_LONG, MPI_MAX, minrank, comm());

        ParallelPrinter printer(comm(), minrank);
        const std::string out =
            identifier() + " Memory Usage" 
             + " min: " + std::to_string(minmem) 
             + " avg: " + std::to_string((long) ((double) totmem/(double) psize)) 
             + " max: " + std::to_string(maxmem)
             + " total: " + std::to_string(totmem);
        printer.echo(out);

        return mem;

    }

    virtual bool is_unstructured() { return false; };
    virtual bool is_amr() { return false; };
    virtual int overset_update_interval() { return 100000000; };
    virtual std::string identifier() { return "ExawindSolver"; }
    virtual MPI_Comm comm() = 0;
    //! Timer names
    const std::vector<std::string> m_names{
        "Pre", "PreConn", "PostConn", "Register", "Update", "Solve", "Post"};
    //! Timers
    Timers m_timers;

protected:
    virtual void init_prolog(bool multi_solver_mode = true) = 0;
    virtual void init_epilog() = 0;
    virtual void prepare_solver_prolog() = 0;
    virtual void prepare_solver_epilog() = 0;
    virtual void pre_advance_stage1() = 0;
    virtual void pre_advance_stage2() = 0;
    virtual void advance_timestep() = 0;
    virtual void post_advance() = 0;
    virtual void pre_overset_conn_work() = 0;
    virtual void post_overset_conn_work() = 0;
    virtual void register_solution() = 0;
    virtual void update_solution() = 0;
};

} // namespace exawind
#endif /* EXAWINDSOLVER_H */
