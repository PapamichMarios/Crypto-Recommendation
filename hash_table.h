#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

#include <string>
#include <algorithm>
#include <fstream>
#include <climits>
#include <ctime>
#include <map>
#include <vector>

#include "hash_node.h"
#include "help_functions.h"
#include "fi.h"
#include "g.h"
#include "utilities.h"

template <typename K>
class HashTable
{
	protected:
    	HashNode<K> **table;
		int tableSize;
		int buckets;

	public:
		HashTable(int tableSize, int k, int dim)
		{
			/*== constructing hash table*/
			this->table = new HashNode<K> *[tableSize]();

			this->tableSize = tableSize;
			this->buckets = 0;
		}
	
		virtual ~HashTable()
		{
			/*== destroying all buckets*/
			HashNode<K> * temp = NULL;
			HashNode<K> * prev = NULL;

			for(int i=0; i<this->tableSize; i++)
			{
				temp = table[i];
				while(temp != NULL)
				{
					prev = temp;
					temp = temp->getNext();
					delete prev;
				}

				table[i] = NULL;
			}

			delete[] table;
		}
		
		virtual int hash(const K &key)=0;
    	virtual void put(const K &key, const K &prenormaliseKey, std::string identifier) =0;

		virtual void recommendationANN(const K &query, int max_neighbours, std::vector<int> &all_neighbours, std::vector<double> &all_distances)=0;
		virtual void recommendationANN_normalised(const K &query, int max_neighbours, std::vector<int> &all_neighbours, std::vector<double> &all_distances)=0;

		virtual void RS(const K &query, int c, double R, std::vector<int>& labels, int cluster) = 0;
		virtual void RS_conflict(const std::vector<K> &queries, int c, double R, std::vector<int>& labels, std::vector<int> clusters) =0;
		virtual void loyds_unassigned(const std::vector<K> &queries, std::vector<int>& labels) =0;

		int getTableSize()
		{
			return this->tableSize;
		}

		void reset_assigned()
		{
			HashNode<K> * temp;

			for(int i=0; i<this->tableSize; i++)
			{
				temp = this->table[i];
				while(temp != NULL)
				{
					/*== reset assigned flag*/
					temp->setAssigned(0);

					/*== iterate to the next hash node*/
					temp = temp->getNext();
				}
			}
		}
};

template <typename K>
class HashTable_EUC : public HashTable<K>
{
	private:
		fi * hash_function;

	public:	
		HashTable_EUC(int tableSize, int k, int dim) : HashTable<K>(tableSize, k, dim)
		{
			/*== construct fi object, used for hashing*/
			this->hash_function = new fi(k, dim);
		}

		~HashTable_EUC()
		{
			delete hash_function;
		}

		int hash(const K &key)
		{
			return hash_function->hashValue(key, this->tableSize);
		}

    	void put(const K &key, const K &prenormalisedKey, std::string identifier)
		{
        	int hash_val = hash_function->hashValue(key, this->tableSize);
        	HashNode<K> *prev = NULL;
        	HashNode<K> *entry = this->table[hash_val];

        	while (entry != NULL) 
			{
            	prev = entry;
            	entry = entry->getNext();
        	}

        	if (entry == NULL) 
			{
				std::string G = hash_function->computeG(key);
            	entry = new HashNode<K>(key, prenormalisedKey, G, identifier);
				this->buckets++;

            	if (prev == NULL) 
				{
                	/*== insert as first bucket*/
                	this->table[hash_val] = entry;
            	} 	
				else 
				{
               		prev->setNext(entry);
            	}
        	} 
    	}

		void recommendationANN(const K &query, int max_neighbours, std::vector<int> &all_neighbours, std::vector<double> &all_distances){ }
		void recommendationANN_normalised(const K &query, int max_neighbours, std::vector<int> &all_neighbours, std::vector<double> &all_distances){ }

		void RS(const K &query, int c, double R, std::vector<int>& labels, int cluster)
		{
			double distance=0;

			int hash_val		= hash_function->hashValue(query, this->tableSize);
			std::string G		= hash_function->computeG(query);
			HashNode<K> * temp  = this->table[hash_val];
			
			/*== iterate through the bucket*/
			while( temp != NULL )
			{
				/*== compare the g(query) with the g(point)*/
				if( G != temp->getG() || temp->getAssigned() == 1)
				{
					temp = temp->getNext();
					continue;
				}
			
				/*== if they match calculate the distance*/
				distance = help_functions::euclidean_distance(query, temp->getKey());
	
				if(distance<c*R)
				{
					labels[stoi(temp->getId())-1] = cluster;
					temp->setAssigned(1);
				}

				/*== iterate to the next node*/
				temp = temp->getNext();	
			}
		}

		void RS_conflict(const std::vector<K> &queries, int c, double R, std::vector<int>& labels, std::vector<int> clusters)
		{
			double distance=0;

			int hash_val		= hash_function->hashValue(queries[0], this->tableSize);
			std::string G		= hash_function->computeG(queries[0]);
			HashNode<K> * temp  = this->table[hash_val];

			/*== iterate through the bucket*/
			while( temp != NULL )
			{
				/*== compare the g(query) with the g(point)*/
				if( G != temp->getG() || temp->getAssigned() == 1)
				{
					temp = temp->getNext();
					continue;
				}
			
				/*== if they match calculate the distance*/
				double min_distance = INT_MAX;
				int cluster;
				for(unsigned int i=0; i<clusters.size(); i++)
				{
					distance = help_functions::euclidean_distance(queries[clusters[i]], temp->getKey());
					
					if(distance<min_distance)
					{
						min_distance = distance;
						cluster = clusters[i];
					}
				}

				if(min_distance<c*R)
				{
					labels[stoi(temp->getId())-1] = cluster;
					temp->setAssigned(1);
				}

				/*== iterate to the next node*/
				temp = temp->getNext();	
			}
		}

		void loyds_unassigned(const std::vector<K> &queries, std::vector<int>& labels)
		{
			HashNode<K> * temp;

			for(int i=0; i<this->tableSize; i++)
			{
				temp = this->table[i];
				while(temp != NULL)
				{
					/*== assign unassigned point*/
					if(temp->getAssigned() == 0)
					{
						double min_distance = INT_MAX;
						double distance;
						int cluster;

						for(unsigned int j=0; j<queries.size(); j++)
						{
							distance = help_functions::euclidean_distance(queries[j], temp->getKey());

							if(distance<min_distance)
							{
								min_distance = distance;
								cluster = j;
							}
						}

						labels[stoi(temp->getId())-1] = cluster;
					}

					/*== reset assigned flag*/
					temp->setAssigned(0);

					/*== iterate to the next hash node*/
					temp = temp->getNext();
				}
			}
		}
};

template <typename K>
class HashTable_COS : public HashTable<K>
{
	private:
		G * hash_function;

	public:
		HashTable_COS(int tableSize, int k, int dim) : HashTable<K>(tableSize, k, dim)
		{
			/*== construct g object, used for hashing*/
			this->hash_function = new G(k, dim);
		}

		~HashTable_COS()
		{
			delete hash_function;
		}

		int hash(const K &key)
		{
			return hash_function->hashValue(key);
		}

    	void put(const K &key, const K &prenormalisedKey, std::string identifier)
		{
        	int hash_val = hash_function->hashValue(key);
        	HashNode<K> *prev = NULL;
        	HashNode<K> *entry = this->table[hash_val];

        	while (entry != NULL) 
			{
            	prev = entry;
            	entry = entry->getNext();
        	}

        	if (entry == NULL) 
			{
            	entry = new HashNode<K>(key, prenormalisedKey, std::to_string(hash_val), identifier);
				this->buckets++;

            	if (prev == NULL) 
				{
                	/*== insert as first bucket*/
                	this->table[hash_val] = entry;
            	} 	
				else 
				{
               		prev->setNext(entry);
            	}
        	} 
			
		}

		void recommendationANN(const K &query, int max_neighbours, std::vector<int> &all_neighbours, std::vector<double> &all_distances)
		{
			std::vector<double> neighbour_distances;
			std::vector<int> neighbour_ids;

			int hash_val = hash_function->hashValue(query);
			HashNode<K> * temp = this->table[hash_val];

			/*== iterate through the bucket & calculate all the distances*/
			while(temp != NULL)
			{
				/*== calculate distance*/
				double distance = help_functions::cosine_distance(query, temp->getPrenormalisedKey());

				neighbour_distances.push_back(distance);
				neighbour_ids.push_back(stoi(temp->getId()));

				temp = temp->getNext();
			}

			for(int i=0; i<max_neighbours; i++)
			{
				if(neighbour_distances.size() <= i)
					break;

				/*== calculate the minimum element of the vector*/
				int index=0;
				double min_distance = INT_MAX;
				for(unsigned int j=0; j<neighbour_distances.size(); j++)
				{
					if(min_distance > neighbour_distances.at(j))
					{
						min_distance = neighbour_distances.at(j);
						index = j;
					}
				}

				/*== assign its id to neighbours vector*/
				all_neighbours.push_back( neighbour_ids.at(index) );
				all_distances.push_back( neighbour_distances.at(index) );

				neighbour_distances.at(index) = INT_MAX;
			}
		}

		void recommendationANN_normalised(const K &query, int max_neighbours, std::vector<int> &all_neighbours, std::vector<double> &all_distances)
		{
			std::vector<double> neighbour_distances;
			std::vector<int> neighbour_ids;

			int hash_val = hash_function->hashValue(query);
			HashNode<K> * temp = this->table[hash_val];

			/*== iterate through the bucket & calculate all the distances*/
			while(temp != NULL)
			{
				/*== if vector is 0 cosine similarity is undefined*/
				if (help_functions::vectorIsZero(temp->getKey()))
				{
					temp = temp->getNext();

					continue;
				}

				/*== calculate distance*/
				double distance = help_functions::cosine_distance(query, temp->getKey());

				neighbour_distances.push_back(distance);
				neighbour_ids.push_back(stoi(temp->getId()));

				temp = temp->getNext();
			}

			for(int i=0; i<max_neighbours; i++)
			{
				if(neighbour_distances.size() <= i)
					break;

				/*== calculate the minimum element of the vector*/
				int index=0;
				double min_distance = INT_MAX;
				for(unsigned int j=0; j<neighbour_distances.size(); j++)
				{
					if(min_distance > neighbour_distances.at(j))
					{
						min_distance = neighbour_distances.at(j);
						index = j;
					}
				}

				/*== assign its id to neighbours vector*/
				all_neighbours.push_back( neighbour_ids.at(index) );
				all_distances.push_back( neighbour_distances.at(index) );

				neighbour_distances.at(index) = INT_MAX;
			}
		}

		void RS(const K &query, int c, double R, std::vector<int>& labels, int cluster)
		{
			double distance=0;

			int hash_val		= hash_function->hashValue(query);
			HashNode<K> * temp  = this->table[hash_val];

			/*== iterate through the bucket*/
			while( temp != NULL )
			{
				if(temp->getAssigned() == 1)
				{
					temp = temp->getNext();
					continue;
				}

				/*== calculate the distance*/
				distance = help_functions::cosine_distance(query, temp->getKey());

				if(distance<c*R)
				{
					labels[stoi(temp->getId())-1] = cluster;
					temp->setAssigned(1);
				}

				/*== iterate to the next node*/
				temp = temp->getNext();	
			}	
		}
		
		void RS_conflict(const std::vector<K> &queries, int c, double R, std::vector<int>& labels, std::vector<int> clusters)
		{
			double distance=0;

			int hash_val		= hash_function->hashValue(queries[0]);
			HashNode<K> * temp  = this->table[hash_val];

			/*== iterate through the bucket*/
			while( temp != NULL )
			{
				if(temp->getAssigned() == 1)
				{
					temp = temp->getNext();
					continue;
				}
			
				/*== calculate the distance*/
				double min_distance = INT_MAX;
				int cluster;
				for(unsigned int i=0; i<clusters.size(); i++)
				{
					distance = help_functions::cosine_distance(queries[clusters[i]], temp->getKey());
					
					if(distance<min_distance)
					{
						min_distance = distance;
						cluster = clusters[i];
					}
				}

				if(min_distance<c*R)
				{
					labels[stoi(temp->getId())-1] = cluster;
					temp->setAssigned(1);
				}

				/*== iterate to the next node*/
				temp = temp->getNext();	
			}
		}

		void loyds_unassigned(const std::vector<K> &queries, std::vector<int>& labels)
		{
			HashNode<K> * temp;

			for(int i=0; i<this->tableSize; i++)
			{
				temp = this->table[i];
				while(temp != NULL)
				{
					/*== assign unassigned point*/
					if(temp->getAssigned() == 0)
					{
						double min_distance = INT_MAX;
						double distance;
						int cluster;

						for(unsigned int j=0; j<queries.size(); j++)
						{
							distance = help_functions::cosine_distance(queries[j], temp->getKey());

							if(distance<min_distance)
							{
								min_distance = distance;
								cluster = j;
							}
						}

						labels[stoi(temp->getId())-1] = cluster;
					}

					/*== reset assigned flag*/
					temp->setAssigned(0);

					/*== iterate to the next hash node*/
					temp = temp->getNext();
				}
			}
			
		}
};

#endif
