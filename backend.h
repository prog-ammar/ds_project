#pragma once


#include<iostream>
#include<vector>
#include<list>
#include<algorithm>
#include<map>
#include<math.h>
#include<sstream>
#include<fstream>
#include<algorithm>

using namespace std;

struct Song
{
    string id;
    string title;
    string artist;
    string genre;
    double loudness;
    double energy;
    double tempo;
    int duration;
    double accoust;
    double danceibility;

    bool operator==(const Song& other) const;
};

struct Node
{
    Node* next;
    Node* prev;
    string song_id;

    Node(string song_id);
};
struct Vertice
{
    int in_degree;
    int out_degree;
    map<string, pair<double, Vertice*>> adj_vertices;
    int data;
    string id;
    double weight;

    Vertice(string id = "", int data = 0);

};

class Recom_Graph
{
private:
    map<string, Vertice*> vertices;

public:

    void add_vertice(string src, string dest, double weight);
    void print(string id);
    vector<string> recommend(string id);
};

struct trienode 
{
    map<char, trienode*> children;
    vector<string> song_ids;
    bool is_end = false;
};

class trie 
{
private:
    trienode* root;

public:
    trie();
    void insert(const string& song_title, const string& song_id);
    vector<string> search(const string& prefix);
};


class Circular_Doubly_Linked_List
{
private:
    Node* head;
    Node* tail;
    Node* curr;

public:
        Circular_Doubly_Linked_List();
        void add_song_at_tail(string song_id);
        string get_curr();
        void add_song_at_curr(string song_id);
        void add_song_at_head(string song_id);
        bool isEmpty();
        void Clear();
        string start_curr();
        string move_curr_front();
        bool id_exists_or_not(string id);
        string move_curr_back();
        vector<string> get_all_nodes();
        string get_prev();
        string get_next();
};

class Player
{
private:
    map<string, Song> songs;
    map<string, vector<string>> playlists_by_genre;
    map<string, vector<string>> playlists_by_artist;
    map<string, vector<string>> user_playlists;
    Circular_Doubly_Linked_List curr_playlist_manager;
    Recom_Graph graph;
    trie song_trie;


    void build_trie();
    void initialize_search();
public:

    Player();
    void add_song_to_current_playlist(string id);
    vector<string> get_current_playlist();
    string start_current_playlist();
    void add_song_to_current_playlist_at_current_position(string id);
    bool song_exists_in_current_playlist(string id);
    void add_current_playlist_at_once(vector<string> ids);
    void clear_current_playlist();
    string play_next_song_of_current_playlist();
    bool any_playlist_playing();
    string get_current_playlist_song_id();
	Song get_song(string id);
    string get_next_playlist_song_id();
    string get_prev_playlist_song_id();
    string play_prev_song_of_current_playlist();
    vector<string> recommend_song_regarding_to_current_song();
    void read_user_playlist(string filename);
    void write_user_playlist(string filename);
    void read_from_file(string filename);
    void add_song_to_user_playlist(string playlist_name, string song_id);
    void remove_song_from_user_playlist(string playlist_name, string song_id);
    map<string, vector<string>> get_genre();
    map<string, vector<string>> get_artist();
    void print_playlist_by_genre();
    void print_playlist_by_artist();
    void print_details();
    void calculate_similarity_of_all_songs();
    void calculate_similarity_of_songs(Song A, Song B);
    const map<string, vector<string>>& get_user_playlists() const;
    vector<string> get_user_playlist(const string& name) const;
    string get_song_title(string id);
    void create_user_playlist(const string& name);
    void delete_user_playlist(const string& name);
    vector<string> search_songs_by_prefix(const string& prefix);
};
