(cat) < 3 > 2
ls | wc
(ls | wc)
(ls) | wc
ls | (wc)
(ls) | (wc)
cat 1 ; ls
ls | grep a && false
ls -al | more
sort < test2.sh | wc
cat test2.sh | wc > count
echo meow > moo2
(echo meow > moo) | diff moo test1.sh> lawl
(echo m; cat test1.sh) ; ls -al
diff a b > f
(ls | grep a && false) && true
cat < /etc/passwd | tr a-z A-Z | sort -u || echo sort failed!
cat < /etc/passwd | tr a-z A-Z | sort -u > out || echo sort failed!
date > newFile
