#include <cstring>
#include <iostream>

template<char... Chars>
struct string_literal1 {
	static constexpr size_t size = sizeof...(Chars);
	static constexpr char string[size + 1]{ Chars..., '\0' };
};

template<typename... Args>
struct tuple;

template<>
struct tuple<> {
	static constexpr size_t size = 0;
};

template<typename T, typename... Args>
struct tuple<T, Args...> {
	T value;
	tuple<Args...> next;
	static constexpr size_t size = 1 + sizeof...(Args);
	tuple() = default;
	tuple(T&& t, Args&&... args)
			: value(std::forward<T>(t)), next(std::forward<Args>(args)...) {}
};

template<typename... Args1, typename... Args2>
tuple<Args1..., Args2...> concat(tuple<Args1...>&& t1, tuple<Args2...>&& t2);

template<typename... Args2>
tuple<Args2...> concat(tuple<>&& t1, tuple<Args2...>&& t2) {
	return t2;
}

template<typename T, typename... Args1, typename... Args2>
tuple<T, Args1..., Args2...> concat(tuple<T, Args1...>&& t1, tuple<Args2...>&& t2) {
	tuple<Args1..., Args2...> tmp = concat(std::forward<tuple<Args1...>>(t1.next), t2);
	tuple<T, Args1..., Args2...> res{ std::forward<T>(t1.value), std::move(tmp) };
	return res;
}

template<size_t N, typename ...Args>
struct get_type_setter;

template<typename T, typename ...Args>
struct get_type_setter<0, T, Args...> {
	using type = T;
};

template<size_t N, typename T, typename ...Args>
struct get_type_setter<N, T, Args...> {
	using type = typename get_type_setter<N - 1, Args...>::type;
};

template<size_t N, typename ...Args>
struct get_struct;

template<typename T, typename ...Args>
struct get_struct<0, T, Args...> {
	static T get_value(tuple<T, Args...> t) {
		return t.value;
	}
};

template<size_t N, typename T, typename ...Args>
struct get_struct<N, T, Args...> {
	static_assert(N > 0);
	typename get_type_setter<N, T, Args...>::type
	static get_value(tuple<T, Args...> t) {
		return get_struct<N - 1, Args...>::get_value(std::forward<tuple<Args...>>(t.next));
	}
};

template<size_t N, typename ...Args>
typename get_type_setter<N, Args...>::type
get(tuple<Args...> t) {
	return get_struct<N, Args...>::get_value(t);
}

template<bool b>
struct boolean_type {
	static constexpr bool value = b;
};

typedef boolean_type<false> FALSE;
typedef boolean_type<true> TRUE;

template<typename T>
struct IS_BOOLEAN {
	static constexpr bool value = std::is_same_v<TRUE, T> || std::is_same_v<FALSE, T>;
};

template<typename T>
struct NOT {
	static constexpr bool value = !T::value;
};

template<typename T, typename U>
struct AND {
	static constexpr bool value = T::value && U::value;
};

template<typename T, typename U>
struct OR {
	static constexpr bool value = T::value || U::value;
};

template<typename T, typename U>
struct XOR {
	static constexpr bool value = T::value ^ U::value;
};

template<bool b, typename F, typename T>
struct IF;

template<typename T, typename F>
struct IF<true, T, F> {
	using type = T;
};

template<typename T, typename F>
struct IF<false, T, F> {
	using type = F;
};

template<size_t N, template<typename> typename FUNC, typename T>
struct WHILE {
	using type = typename WHILE<N - 1, FUNC, typename FUNC<T>::type>::type;
};

template<template<typename> typename FUNC, typename T>
struct WHILE<0, FUNC, T> {
	using type = T;
};

template<typename T>
struct TMP_FUNC {
	using type = tuple<T>;
};

template<typename T, typename... Args>
struct TMP_FUNC<tuple<T, Args...>> {
	using type = tuple<T, T, Args...>;
};

template<typename T>
struct NATURAL;

template<>
struct NATURAL<void> {
	static constexpr size_t value = 0;
};

template<typename T>
struct NATURAL<NATURAL<T>> {
	static constexpr size_t value = 1 + NATURAL<T>::value;
};

typedef NATURAL<void> NATURAL_ZERO;
typedef NATURAL<NATURAL_ZERO> NATURAL_ONE;
typedef NATURAL<NATURAL_ONE> NATURAL_TWO;

template<typename T, typename U>
struct NATURAL_ADD;

template<typename T>
struct NATURAL_ADD<NATURAL<T>, NATURAL_ZERO> {
	using type = NATURAL<T>;
};

template<typename T, typename U>
struct NATURAL_ADD<NATURAL<T>, NATURAL<U>> {
	using type = typename NATURAL_ADD<NATURAL<NATURAL<T>>, U>::type;
};

template<typename Torig, typename T, typename U>
struct MULTIPLY_HELPER;

template<typename Torig, typename T>
struct MULTIPLY_HELPER<Torig, NATURAL<T>, NATURAL_ZERO> {
	using type = NATURAL<T>;
};

template<typename Torig, typename T, typename U>
struct MULTIPLY_HELPER<Torig, NATURAL<T>, NATURAL<U>> {
	using type = typename MULTIPLY_HELPER<Torig, typename NATURAL_ADD<NATURAL<T>, Torig>::type, U>::type;
};

template<typename T, typename U>
struct NATURAL_MULTIPLY {
	using type = typename MULTIPLY_HELPER<T, NATURAL_ZERO, U>::type;
};

template<typename T, typename U>
struct LESS;

template<typename T>
struct LESS<NATURAL<T>, NATURAL_ZERO> {
	static constexpr bool value = false;
};

template<typename U>
struct LESS<NATURAL_ZERO, NATURAL<NATURAL<U>>> {
	static constexpr bool value = true;
};

template<typename T, typename U>
struct LESS<NATURAL<T>, NATURAL<U>> {
	static constexpr bool value = LESS<T, U>::value;
};

template<typename T, typename U>
struct GREATER {
	static constexpr bool value = LESS<U, T>::value;
};

template<typename T, typename U>
struct INTEGER;

template<typename T>
struct INTEGER<NATURAL<T>, NATURAL_ZERO> {
	using type = INTEGER<NATURAL<T>, NATURAL_ZERO>;
	static constexpr ssize_t value = NATURAL<T>::value;
};

template<typename U>
struct INTEGER<NATURAL_ZERO, NATURAL<U>> {
	using type = INTEGER<NATURAL_ZERO, NATURAL<U>>;
	static constexpr ssize_t value = -INTEGER<NATURAL<U>, NATURAL_ZERO>::value;
};

template<typename T, typename U>
struct INTEGER<NATURAL<NATURAL<T>>, NATURAL<NATURAL<U>>> {
	using type = typename INTEGER<NATURAL<T>, NATURAL<U>>::type;
	static constexpr ssize_t value = type::value;
};

typedef INTEGER<NATURAL_ZERO, NATURAL_ZERO> INTEGER_ZERO;
typedef INTEGER<NATURAL_ONE, NATURAL_ZERO> INTEGER_ONE;
typedef INTEGER<NATURAL_TWO, NATURAL_ZERO> INTEGER_TWO;
typedef INTEGER<NATURAL_ZERO, NATURAL_ONE> INTEGER_MINUS_ONE;
typedef INTEGER<typename NATURAL_ADD<NATURAL_ONE, NATURAL_TWO>::type, NATURAL_ZERO> INTEGER_THREE;

template<typename T>
struct NEGATE;

template<typename T, typename U>
struct NEGATE<INTEGER<T, U>> {
	using type = typename INTEGER<U, T>::type;
};

template<typename T, typename U>
struct INTEGER_ADD;

template<typename T1, typename U1, typename T2, typename U2>
struct INTEGER_ADD<INTEGER<T1, U1>, INTEGER<T2, U2>> {
	using type = typename INTEGER<typename NATURAL_ADD<T1, T2>::type, typename NATURAL_ADD<U1, U2>::type>::type;
};

template<typename T, typename U>
struct INTEGER_MINUS {
	using type = typename INTEGER_ADD<T, typename NEGATE<U>::type>::type;
};

template<typename T, typename U>
struct INTEGER_MULTIPLY;

template<typename T1, typename U1, typename T2, typename U2>
struct INTEGER_MULTIPLY<INTEGER<T1, U1>, INTEGER<T2, U2>> {
	using type = typename INTEGER<typename NATURAL_ADD<typename NATURAL_MULTIPLY<T1, T2>::type, typename NATURAL_MULTIPLY<U1, U2>::type>::type,
			typename NATURAL_ADD<typename NATURAL_MULTIPLY<T1, U2>::type, typename NATURAL_MULTIPLY<U1, T2>::type>::type>::type;
};

int main() {
	std::cout << "Hello, world!" << std::endl;
	std::cout << sizeof(string_literal1<>::string) << std::endl;
	std::cout << string_literal1<'H'>::string << std::endl;
	std::cout << string_literal1<'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!'>::string << std::endl;
	tuple<int, char, int, int> t1(1, 'h', 100, 42);
	std::cout << get_struct<2, int, char, int, int>::get_value(t1) << std::endl;
	std::cout << get<0>(t1) << std::endl;
	std::cout << static_cast<typename IF<FALSE::value, int, char>::type>('a') << std::endl;
	std::cout << WHILE<142, TMP_FUNC, int>::type::size << std::endl;
	std::cout << NATURAL_ADD<NATURAL_ADD<NATURAL_ONE, NATURAL_TWO>::type, NATURAL_ZERO>::type::value << std::endl;
	std::cout << NATURAL_MULTIPLY<NATURAL_TWO, NATURAL_MULTIPLY<NATURAL_TWO, NATURAL_TWO>::type>::type::value << std::endl;
	std::cout << LESS<NATURAL_TWO, NATURAL_TWO>::value << std::endl;
	std::cout << NEGATE<INTEGER_TWO>::type::value << std::endl;
	std::cout << INTEGER_MINUS<INTEGER_ONE, typename NEGATE<INTEGER_TWO>::type>::type::value << std::endl;
	std::cout << INTEGER_MULTIPLY<INTEGER_THREE, typename NEGATE<INTEGER_TWO>::type>::type::value << std::endl;
}
