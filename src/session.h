/*
 * Copyright (c) 2003-2009, John Wiegley.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of New Artisans LLC nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @defgroup report Reporting
 */

/**
 * @file   session.h
 * @author John Wiegley
 *
 * @ingroup report
 *
 * @brief Brief
 *
 * Long.
 */
#ifndef _SESSION_H
#define _SESSION_H

#include "scope.h"
#include "handler.h"
#include "journal.h"
#include "account.h"
#include "format.h"

namespace ledger {

/**
 * @brief Brief
 *
 * Long.
 */
class session_t : public noncopyable, public scope_t
{
  friend void set_session_context(session_t * session);

public:
  bool flush_on_next_data_file;
  int  current_year;

  shared_ptr<commodity_pool_t>	commodity_pool;
  ptr_list<journal_t::parser_t> parsers;
  ptr_list<journal_t>		journals;
  scoped_ptr<account_t>		master;

  explicit session_t();
  virtual ~session_t() {
    TRACE_DTOR(session_t);
  }

  void now_at_command_line(const bool truth) {
    flush_on_next_data_file = true;
  }

  journal_t * create_journal();
  void        close_journal(journal_t * journal);

  std::size_t read_journal(journal_t&	 journal,
			   std::istream& in,
			   const path&	 pathname,
			   account_t *   master = NULL);
  std::size_t read_journal(journal_t&	 journal,
			   const path&	 pathname,
			   account_t *   master = NULL);

  std::size_t read_data(journal_t&    journal,
			const string& master_account = "");

  void register_parser(journal_t::parser_t * parser) {
    parsers.push_back(parser);
  }
  void unregister_parser(journal_t::parser_t * parser);

  void clean_xacts();
  void clean_xacts(entry_t& entry);
  void clean_accounts();
  void clean_all() {
    clean_xacts();
    clean_accounts();
  }

  virtual expr_t::ptr_op_t lookup(const string& name);

  /**
   * Option handlers
   */

  OPTION(session_t, account_); // -a
  OPTION(session_t, download); // -Q

  OPTION__
  (session_t, file_, // -f
   std::list<path> data_files;
   CTOR(session_t, file_) {}
   DO_(args) {
     assert(args.size() == 1);
     if (parent->flush_on_next_data_file) {
       data_files.clear();
       parent->flush_on_next_data_file = false;
     }
     data_files.push_back(args[0].as_string());
   });

  OPTION(session_t, input_date_format_);
  OPTION(session_t, price_db_);
};

/**
 * Set the current session context, transferring all static globals to point
 * at the data structures related to this session.  Although Ledger itself is
 * not thread-safe, by locking, switching session context, then unlocking
 * after an operation is done, multiple threads can sequentially make use of
 * the library.  Thus, a session_t maintains all of the information relating
 * to a single usage of the Ledger library.
 */
void set_session_context(session_t * session);

} // namespace ledger

#endif // _SESSION_H
