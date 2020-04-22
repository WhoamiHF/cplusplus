#ifndef GRAPH_DB_HPP
#define GRAPH_DB_HPP
#include <tuple>
#include <vector>
#include <utility>
#include<string>
#include <unordered_map>

template<typename t>
class tupleToColumns;

template<class GraphSchema>
class graph_db;

template<typename ... Ts>
class tupleToColumns<std::tuple<Ts ...>> {
public:
	using sheet_t = std::tuple<std::vector<Ts> ...>;

	template<size_t I>
	using type_column = std::tuple_element_t<I, std::tuple<Ts...>>;

	template<size_t I>
	decltype(auto) get(size_t index)
	{
		return std::get<I>(t)[index];
	}

	auto getRow(size_t index) {
		return getRow2(std::make_index_sequence<sizeof ... (Ts)>(), index);
	}

	void setRow(size_t index, Ts ... values) {
		setRow2(std::make_index_sequence<sizeof ... (Ts)>(), index, values ...);
	}
	template<size_t ... sq>
	void setRow2(std::index_sequence<sq ...>, size_t index, Ts ... values) {
		(set<sq>(index, values), ...);
	}

	template<size_t ... sq>
	auto getRow2(std::index_sequence<sq ...>, size_t index) {
		return std::tuple{ get<sq>(index)... };
	}

	//reference - &,
	template<size_t I>
	void set(size_t index, type_column<I> element)
	{
		std::get<I>(t)[index] = element;
	}

	void add(Ts... columns)
	{
		add2(columns ..., std::make_index_sequence<sizeof ... (Ts)>());
	}
	template<size_t ... sq>
	void addE(std::index_sequence<sq ...>)
	{
		(std::get<sq>(t).push_back(type_column<sq>()), ...);
	}
private:
	sheet_t t;

	template<size_t ... sq>
	void add2(Ts... columns, std::index_sequence<sq ...>)
	{
		(std::get<sq>(t).push_back(columns), ...);
	}
};
template<class GraphSchema>
class edge_class_t;

template<class GraphSchema>
class vertex_class_t;

template<class GraphSchema>
class edge_it;

template<class GraphSchema>
class vertex_it;

template<class GraphSchema>
class edges_class_t {
public:
	edges_class_t(graph_db<GraphSchema>& database_) :database(database_) {}
	friend graph_db<GraphSchema>;
	friend edge_class_t<GraphSchema>;
	friend edge_it<GraphSchema>;

private:
	tupleToColumns<typename GraphSchema::edge_property_t> properties;
	graph_db<GraphSchema>& database;
	std::vector<typename GraphSchema::edge_user_id_t> indexToID;
	//std::unordered_map<eUser_t, size_t> IDs;
	std::vector<size_t> startVertices;
	std::vector<size_t> endVertices;
};


template<class GraphSchema>
class edge_class_t {
public:
	edge_class_t(size_t index_, edges_class_t<GraphSchema>& edges_) :index(index_), edges(edges_) {}
	/**
   * @brief Returns the immutable user id of the element.
   */
	auto id() const {
		return edges.indexToID[index];
	}

	/**
	 * @brief Returns all immutable properties of the element in tuple.
	 * @note The return type is GraphSchema::vertex_property_t for vertexes and GraphSchema::edge_property_t for edges.
	 */
	auto get_properties() const {
		return edges.properties.getRow(index);
	}

	/**
	 *
	 * @brief Returns a single immutable property of the I-th element.
	 * @tparam I An index of the property.
	 * @return The value of the property.
	 * @note The first property is on index 0.
	 */
	template<size_t I>
	decltype(auto) get_property() const {
		return  edges.properties.template get<I>(index);
	}

	/**
	 * @brief Sets the values of properties of the element.
	 * @tparam PropsType Types of the properties.
	 * @param props The value of each individual property.
	 * @note Should not compile if not provided with all properties.
	 */
	template<typename ...PropsType>
	void set_properties(PropsType&&...props) {
		edges.properties.setRow(index, props ...);
	}

	/**
	 * @brief Set a value of the given property of the I-th element
	 * @tparam I An index of the property.
	 * @tparam PropType The type of the property.
	 * @param prop The new value of the property.
	 * @note The first property is on index 0.
	 */
	template<size_t I, typename PropType>
	void set_property(const PropType& prop) {
		edges.properties.template set<I>(index, prop);
	}

	/**
	 * @brief Returns the source vertex of the edge.                                   with
	 * @return The vertex.
	 */
	auto src() const {
		size_t tmpIndex = edges.startVertices[index];
		return edges.database.getVertex(tmpIndex);
	}

	/**
	 * @brief Returns the destination vertex of the edge.
	 * @return The vertex.
	 */
	auto dst() const {
		size_t tmpIndex = edges.endVertices[index];
		return edges.database.getVertex(tmpIndex);
	}
private:
	size_t index;
	edges_class_t<GraphSchema>& edges;
};

template<class GraphSchema>
class vertex_class_t;

template<class GraphSchema>
class vertices_class_t {
public:
	friend vertex_it<GraphSchema>;
	friend graph_db<GraphSchema>;
	vertices_class_t(graph_db<GraphSchema>& database_) :database(database_) {}
	friend vertex_class_t<GraphSchema>;
	std::vector<std::vector<size_t>> neighbors;
private:
	tupleToColumns<typename GraphSchema::vertex_property_t> properties;
	std::vector<typename GraphSchema::vertex_user_id_t> indexToID;
	graph_db<GraphSchema>& database;
};

template<class pointed>
class iterator;

template<class GraphSchema>
class vertex_class_t {
public:
	friend vertices_class_t<GraphSchema>;
	friend graph_db<GraphSchema>;
	vertex_class_t(size_t index_, edges_class_t<GraphSchema>& edges_, vertices_class_t<GraphSchema>& vertices_) :
		index(index_), edgs(edges_), vertices(vertices_) {}
	/**
	 * @brief Returns the immutable user id of the element.
	 */
	auto id() const {
		return vertices.indexToID[index];
	}

	/**
	 * @brief Returns all immutable properties of the element in tuple.
	 * @note The return type is GraphSchema::vertex_property_t for vertexes and GraphSchema::edge_property_t for edges.
	 */
	auto get_properties() const {
		return vertices.properties.getRow(index);
	}

	/**
	 *
	 * @brief Returns a single immutable property of the I-th element.
	 * @tparam I An index of the property.
	 * @return The value of the property.
	 * @note The first property is on index 0.
	 */
	template<size_t I>
	decltype(auto) get_property() const {
		return vertices.properties.template get<I>(index);
	}

	/**
	 * @brief Sets the values of properties of the element.
	 * @tparam PropsType Types of the properties.
	 * @param props The value of each individual property.
	 * @note Should not compile if not provided with all properties.
	 */
	template<typename ...PropsType>
	void set_properties(PropsType&&...props) {
		vertices.properties.setRow(index, props ...);
	}

	/**
	 * @brief Set a value of the given property of the I-th element
	 * @tparam I An index of the property.
	 * @tparam PropType The type of the property.
	 * @param prop The new value of the property.
	 * @note The first property is on index 0.
	 */
	template<size_t I>
	void set_property(std::tuple_element_t<I, typename GraphSchema::vertex_property_t> prop) {
		vertices.properties.template set<I>(index, prop);
	}

	/**
	 * @see graph_db::neighbor_it_t
	 */
	class neighbor_it_t {
	public:
		neighbor_it_t(std::vector<size_t>& object_, size_t position_, vertices_class_t<GraphSchema>& vertices_) :object(object_), position(position_), vertices(vertices_) {}
		neighbor_it_t(const neighbor_it_t& other) :object(other.object), position(other.position), vertices(other.vertices) {}
		neighbor_it_t operator=(const neighbor_it_t& other) const {
			object = other.object;
			position = other.position;
			vertices = other.vertices;
			return *this;
		}
		void swap(neighbor_it_t& other) {
			auto tmp = other;
			other = *this;
			*this = tmp;
		}
		edge_class_t<GraphSchema> operator*() {
			size_t ind = object[position];
			return vertices.database.getEdge(ind);
		}

		bool operator==(const neighbor_it_t& other) const {
			if (&(this->object) == &(other.object)) {
				if (other.position < other.object.size()) {
					return other.position == this->position;
				}
				else {
					return (this->position >= this->object.size());
				}
			}
			else {
				return false;
			}
		}
		bool operator!=(const neighbor_it_t& other) const {
			return !(*this == other);
		}
		neighbor_it_t& operator++() { position++; return *this; }
		neighbor_it_t operator++(int) {
			neighbor_it_t temp = *this;
			++* this;
			return temp;
		}
		size_t position;
		std::vector<size_t>& object;
		vertices_class_t<GraphSchema>& vertices;
	};



	/**
	 * @brief Returns begin() and end() iterators to all forward edges from the vertex
	 * @return A pair<begin(), end()> of a neighbor iterators.
	 * @see graph_db::neighbor_it_t
	 */

	std::pair<neighbor_it_t, neighbor_it_t> edges() const {
		neighbor_it_t beg(const_cast<std::vector<size_t>&>(vertices.neighbors[index]), 0, const_cast<vertices_class_t<GraphSchema>&>(vertices));
		neighbor_it_t fin(const_cast<std::vector<size_t>&>(vertices.neighbors[index]), vertices.neighbors[index].size(), const_cast<vertices_class_t<GraphSchema>&>(vertices));
		return std::make_pair(beg, fin);
	}

private:
	edges_class_t<GraphSchema>& edgs;
	size_t index;
	vertices_class_t<GraphSchema>& vertices;
};

template<typename GraphSchema>
class vertex_it {
public:
	vertex_it(graph_db<GraphSchema>* graph_, size_t position_) :graph(graph_), position(position_) {}
	vertex_it(const vertex_it<GraphSchema>& other):position(other.position),graph(other.graph){}
	vertex_it<GraphSchema> operator=(const vertex_it<GraphSchema>& other) const {
		position = other.position;
		graph = other.graph;
		return *this;
	}
	void swap(vertex_it<GraphSchema>& other) {
		auto tmp = other;
		other = *this;
		*this = tmp;
	}
	vertex_class_t<GraphSchema> operator*() {
		return graph->getVertex(position);
	}

	bool operator==(const vertex_it<GraphSchema>& other) const {
		if (this->graph == other.graph) {
			if (other.position < graph->vertices.indexToID.size()){
				return other.position == this->position;
			}
			else {
				return (this->position >= graph->vertices.indexToID.size());
			}
		}
		else {
			return false;
		}
	}
	bool operator!=(const vertex_it<GraphSchema>& other) const {
		return !(*this == other);
	}
	vertex_it<GraphSchema>& operator++() { position++; return *this; }
	vertex_it<GraphSchema> operator++(int) {
		vertex_it<GraphSchema> temp = *this;
		++* this;
		return temp;
	}
	size_t position;
	graph_db<GraphSchema>* graph;
};

template<typename GraphSchema>
class edge_it {
public:
	edge_it(graph_db<GraphSchema>* graph_, size_t position_) :graph(graph_), position(position_) {}
	edge_it(const edge_it<GraphSchema>& other) :position(other.position), graph(other.graph) {}
	edge_it<GraphSchema> operator=(const edge_it<GraphSchema>& other) const {
		position = other.position;
		graph = other.graph;
		return *this;
	}
	void swap(edge_it<GraphSchema>& other) {
		auto tmp = other;
		other = *this;
		*this = tmp;
	}
	edge_class_t<GraphSchema> operator*() {
		return graph.getEdge(position);
	}

	bool operator==(const edge_it<GraphSchema>& other) const {
		if (this.graph == other.graph) {
			if (other.position < graph.edges.indexToID.size()) {
				return other.position == this->position;
			}
			else {
				return (this->position >= this->graph.edges.indexToID.size());
			}
		}
		else {
			return false;
		}
	}
	bool operator!=(const edge_it<GraphSchema>& other) const {
		return !(*this == other);
	}
	edge_it<GraphSchema>& operator++() { position++; return *this; }
	edge_it<GraphSchema> operator++(int) {
		edge_it<GraphSchema> temp = *this;
		++* this;
		return temp;
	}
	size_t position;
	graph_db<GraphSchema>* graph;
};

/**
 * @brief A graph database that takes its schema (types and number of vertex/edge properties, user id types) from a given trait
 * @tparam GraphSchema A trait which specifies the schema of the graph database.
 * @see graph_schema
 */
template<class GraphSchema>
class graph_db {
public:
	friend vertex_class_t<GraphSchema>;//
	friend vertices_class_t<GraphSchema>;
	friend edges_class_t<GraphSchema>;//
	friend vertex_it<GraphSchema>;
	friend edge_it<GraphSchema>;
	graph_db() :edges(edges_class_t(*this)), vertices(vertices_class_t(*this)) {}
	/**
	 * @brief A type representing a vertex.
	 * @see vertex
	 */
	using vertex_t = vertex_class_t<GraphSchema>;
	/**
	 * @brief A type representing an edge.
	 * @see edge
	 */
	using edge_t = edge_class_t<GraphSchema>;
	vertex_t getVertex(size_t index) {
		return vertex_t(index, edges, vertices);
	}

	edge_t getEdge(size_t index) {
		return edge_t(index, edges);
	}

	/**
	 * @brief A type representing a vertex iterator. Must be at least of output iterator. Returned value_type is a vertex.
	 * @note Iterate in insertion order.
	 */
	using vertex_it_t = vertex_it<GraphSchema>;

	/**
	 * @brief A type representing a edge iterator. Must be at least an output iterator. Returned value_type is an edge.
	 * @note Iterate in insertion order.
	 */
	using edge_it_t = edge_it<GraphSchema>;

	/**
	 * @brief A type representing a neighbor iterator. Must be at least an output iterator. Returned value_type is an edge.
	 * @note Iterate in insertion order.
	 */
	using neighbor_it_t = typename vertex_t::neighbor_it_t;

	/**
	 * @brief Insert a vertex into the database.
	 * @param vuid A user id of the newly created vertex.
	 * @return The newly created vertex.
	 * @note The vertex's properties have default values.                sss
	 */
	vertex_t add_vertex(typename GraphSchema::vertex_user_id_t&& vuid) {
		vertices.indexToID.push_back(std::move(vuid));
		vertices.neighbors.push_back(std::vector<size_t>());
		vertices.properties.addE(std::make_index_sequence<std::tuple_size<typename GraphSchema::vertex_property_t>::value>());
		return vertex_t(vertices.indexToID.size() - 1, edges, vertices);
	}
	vertex_t add_vertex(const typename GraphSchema::vertex_user_id_t& vuid) {
		vertices.indexToID.push_back(vuid);
		vertices.neighbors.push_back(std::vector<size_t>());
		vertices.properties.addE(std::make_index_sequence<std::tuple_size<typename GraphSchema::vertex_property_t>::value>());
		return vertex_t(vertices.indexToID.size() - 1, edges, vertices);
	}

	/**
	 * @brief Insert a vertex into the database with given values of the vertex's properties.
	 * @tparam Props All types of properties.
	 * @param vuid A user id of the newly created vertex.
	 * @param props Properties of the new vertex.
	 * @return The newly created vertex.
	 * @note Should not compile if not provided with all properties.
	 */
	template<typename ...Props>
	vertex_t add_vertex(typename GraphSchema::vertex_user_id_t&& vuid, Props&&...props) {
		vertices.indexToID.push_back(std::move(vuid));
		vertices.neighbors.push_back(std::vector<size_t>());
		vertices.properties.add(props ...);
		return vertex_t(vertices.indexToID.size() - 1, edges, vertices);
	}
	template<typename ...Props>
	vertex_t add_vertex(const typename GraphSchema::vertex_user_id_t& vuid, Props&&...props) {
		vertices.indexToID.push_back(vuid);
		vertices.neighbors.push_back(std::vector<size_t>());
		vertices.properties.add(props ...);
		return vertex_t(vertices.indexToID.size() - 1, edges, vertices);
	}

	/**
	 * @brief Returns begin() and end() iterators to all vertexes in the database.
	 * @return A pair<begin(), end()> of vertex iterators.
	 * @note The iterator can iterate in any order.
	 */
	std::pair<vertex_it_t, vertex_it_t> get_vertexes() const {
		return std::make_pair(vertex_it(const_cast<graph_db<GraphSchema>*>(this), 0), vertex_it(const_cast<graph_db<GraphSchema>*>(this), vertices.indexToID.size()));

	}

	/**
	 * @brief Insert a directed edge between v1 and v2 with a given user id.
	 * @param euid An user id of the edge.
	 * @param v1 A source vertex of the edge.
	 * @param v2 A destination vertex of the edge.
	 * @return The newly create edge.
	 * @note The edge's properties have default values.
	 */
	edge_t add_edge(typename GraphSchema::edge_user_id_t&& euid, const vertex_t& v1, const vertex_t& v2) {
		edges.indexToID.push_back(euid);
		edges.startVertices.push_back(v1.index);
		edges.endVertices.push_back(v2.index);
		edges.properties.addE(std::make_index_sequence<std::tuple_size<typename GraphSchema::edge_property_t>::value>());
		vertices.neighbors[v1.index].push_back(edges.indexToID.size() - 1);
		return edge_t(edges.indexToID.size() - 1, edges);
	}
	edge_t add_edge(const typename GraphSchema::edge_user_id_t& euid, const vertex_t& v1, const vertex_t& v2) {
		edges.indexToID.push_back(std::forward(euid));
		edges.startVertices.push_back(v1.index);
		edges.endVertices.push_back(v2.index);
		edges.properties.addE(std::make_index_sequence<std::tuple_size<typename GraphSchema::edge_property_t>::value>());
		vertices.neighbors[v1.index].pushback(edges.indexToID.size() - 1);
		return edge_t(edges.indexToID.size() - 1, edges);
	}

	/**
	 * @brief Insert a directed edge between v1 and v2 with a given user id and given properties.
	 * @tparam Props Types of properties
	 * @param euid An user id of the edge.
	 * @param v1 A source vertex of the edge.
	 * @param v2 A destination vertex of the edge.
	 * @param props All properties of the edge.
	 * @return The newly create edge.
	 * @note Should not compile if not provided with all properties.
	 */
	template<typename ...Props>
	edge_t add_edge(typename GraphSchema::edge_user_id_t&& euid, const vertex_t& v1, const vertex_t& v2, Props&&...props) {
		edges.indexToID.push_back(euid);
		edges.startVertices.push_back(v1.index);
		edges.endVertices.push_back(v2.index);
		edges.properties.add(props ...);
		vertices.neighbors[v1.index].push_back(edges.indexToID.size() - 1);
		return edge_t(edges.indexToID.size() - 1, edges);
	}
	template<typename ...Props>
	edge_t add_edge(const typename GraphSchema::edge_user_id_t& euid, const vertex_t& v1, const vertex_t& v2, Props&&...props) {
		edges.indexToID.push_back(forward(euid));
		edges.startVertices.push_back(v1.index);
		edges.endVertices.push_back(v2.index);
		edges.properties.add(props ...);
		vertices.neighbors[v1.index].pushback(edges.indexToID.size() - 1);
		return edge_t(edges.indexToID.size() - 1, edges);
	}

	/**
	 * @brief Returns begin() and end() iterators to all edges in the database.
	 * @return A pair<begin(), end()> of edge iterators.
	 * @note The iterator can iterate in any order.
	 */
	std::pair<edge_it_t, edge_it_t> get_edges() const {
		return std::make_pair(edge_it(const_cast<graph_db<GraphSchema>*>(this), 0), edge_it(const_cast<graph_db<GraphSchema>*>(this), edges.indexToID.size()));
		
	}
private:
	edges_class_t<GraphSchema> edges;
	vertices_class_t<GraphSchema> vertices;
};

#endif //GRAPH_DB_HPP
