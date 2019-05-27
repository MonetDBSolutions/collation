#!/usr/bin/env bash
echo LIKE
for i in {1..10}; do mclient -f rowcount -t performance -s "select count(*) from orders where o_comment like '%request%';" 2>&1 | grep clk; done
echo ILIKE
for i in {1..10}; do mclient -f rowcount -t performance -s "select count(*) from orders where o_comment ilike '%request%';" 2>&1 | grep clk; done
echo collationlike
for i in {1..10}; do mclient -f rowcount -t performance -s "select count(*) from orders where collationlike(o_comment, '%request%', 'en_US');" 2>&1 | grep clk; done

mclient -f raw -s "trace select count(*) from orders where collationlike(o_comment, '%request%', 'en_US');"
