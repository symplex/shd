//
// Copyright 2014 Ettus Research LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef INCLUDED_TEST_GRAPH_HPP
#define INCLUDED_TEST_GRAPH_HPP

#include <shd/rfnoc/node_ctrl_base.hpp>
#include <shd/rfnoc/sink_node_ctrl.hpp>
#include <shd/rfnoc/source_node_ctrl.hpp>

#define MAKE_NODE(name) test_node::sptr name(new test_node(#name));

// Smallest possible test class
class test_node : virtual public shd::rfnoc::sink_node_ctrl, virtual public shd::rfnoc::source_node_ctrl
{
public:
    typedef boost::shared_ptr<test_node> sptr;

    test_node(const std::string &test_id) : _test_id(test_id) {};

    void issue_stream_cmd(const shd::stream_cmd_t &, const size_t) {/* nop */};

    std::string get_test_id() const { return _test_id; };

private:
    const std::string _test_id;

}; /* class test_node */

void connect_nodes(shd::rfnoc::source_node_ctrl::sptr A, shd::rfnoc::sink_node_ctrl::sptr B)
{
    const size_t actual_src_port = A->connect_downstream(B);
    const size_t actual_dst_port = B->connect_upstream(A);
    A->set_downstream_port(actual_src_port, actual_dst_port);
    B->set_upstream_port(actual_dst_port, actual_src_port);
}

#endif /* INCLUDED_TEST_GRAPH_HPP */
// vim: sw=4 et:
