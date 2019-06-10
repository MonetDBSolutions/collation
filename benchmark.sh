#!/usr/bin/env bash
echo LIKE
for i in {1..10}; do mclient -f rowcount -t performance -s "select count(*) from orders where o_comment like '%request%';" 2>&1 | grep clk; done
echo ILIKE
for i in {1..10}; do mclient -f rowcount -t performance -s "select count(*) from orders where o_comment ilike '%request%';" 2>&1 | grep clk; done
echo "rematch(.., to_re(...))"
for i in {1..10}; do mclient -f rowcount -t performance -s "select count(*) from orders where rematch(o_comment, to_re('%request%'));" 2>&1 | grep clk; done
echo collationlike
for i in {1..10}; do mclient -f rowcount -t performance -s "select count(*) from orders where collationlike(o_comment, '%request%', 'en_US');" 2>&1 | grep clk; done

mclient -f raw -s "trace select count(*) from orders where collationlike(o_comment, '%request%', 'en_US');"


#mclient -s "CREATE TABLE ORDERS  ( O_ORDERKEY       INTEGER NOT NULL, O_CUSTKEY        INTEGER NOT NULL, O_ORDERSTATUS    CHAR(1) NOT NULL, O_TOTALPRICE     DECIMAL(15,2) NOT NULL, O_ORDERDATE      DATE NOT NULL, O_ORDERPRIORITY  CHAR(15) NOT NULL, O_CLERK          CHAR(15) NOT NULL, O_SHIPPRIORITY   INTEGER NOT NULL, O_COMMENT        VARCHAR(79) NOT NULL);"
# mclient -s "COPY 15000000 RECORDS INTO orders from '/home/aris/sources/collation/orders.tbl' USING DELIMITERS '|', '|\n' LOCKED;"