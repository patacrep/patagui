#!/bin/bash
# small script to regenerate feed.xml for nightly update base on 
# a folder with all the '<embeded>' balise with date.

# we should
# ouput the Headers,
# for each xxx.feed
# subsectionning, 
# including the xxx.feed
# close sub
# footers.

print_headers(){
cat daily/atom.xml.head 
}
print_footers(){
cat daily/atom.xml.footer 
}
print_opensec(){ 
cat daily/atom.xml.opensec
}
print_closesec(){ 
cat daily/atom.xml.closesec 
}

gen_feed(){
print_headers

for i in daily/*.feed 
do
	print_opensec
	cat $i|grep -v OpenSSL
	print_closesec
done
print_footers
}

gen_feed


