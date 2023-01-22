

execute:

	python3 ./testcases/script.py

	g++ ./hash_tables/chaining.cpp -o ./hash_tables/chaining
	./hash_tables/chaining < ./testcases/testcases.txt > ./hash_tables/dump.txt
	rm ./hash_tables/chaining

	g++ ./hash_tables/openaddressing.cpp -o ./hash_tables/openaddressing
	./hash_tables/openaddressing < ./testcases/testcases.txt > ./hash_tables/dump2.txt
	rm ./hash_tables/openaddressing

	g++ ./std_tables/hash_table.cpp -o ./std_tables/std_hash_table
	./std_tables/std_hash_table < ./testcases/testcases.txt > ./std_tables/dump.txt
	rm ./std_tables/std_hash_table

	g++ ./submaps/submap_chaining.cpp -lpthread -o ./submaps/submap_chainings
	./submaps/submap_chainings < ./testcases/testcases.txt
	rm ./submaps/submap_chainings