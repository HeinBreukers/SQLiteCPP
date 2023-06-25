// #include "RootNode.hpp"
// //#include "Pager.hpp"
// #include "InternalNode.hpp"
// #include "LeafNode.hpp"

// // RootNode::operator bool () const 
// // { 
// //     return IsNotNull();
// // }  

// RootNode& RootNode::copyFrom(const RootNode& src)
// {
//     RootNode::copy(src.ptr,ptr);
//     return *this;
// }

// // void RootNode::print(Pager* pager) const
// // {
// //     std::visit([&pager](auto&& t_ptr)->void
// //     {
// //         t_ptr->print(pager);
// //     },ptr);
// // }

// // [[nodiscard]] NodeType RootNode::nodeType() const
// // {
// //     return std::visit([](auto&& t_ptr)
// //     {
// //         return t_ptr->m_header.m_nodeType;
// //     },ptr);
// // }

// // [[nodiscard]] std::array<char,PAGE_SIZE>* RootNode::toBytes()
// // {
// //     return std::visit([](auto&& t_ptr)
// //     {
// //         return t_ptr->toBytes();
// //     },ptr);
// // }


// // [[nodiscard]] uint32_t RootNode::maxKey()
// // {
// //     return std::visit([](auto&& t_ptr) ->uint32_t
// //     {
// //         return t_ptr->maxKey();
// //     },ptr);
// // }

// void RootNode::copy(const nodeVariant& src, nodeVariant& dest)
// {
//     std::visit([](auto&& t_src, auto&& t_dest)
//     {
//         if constexpr (std::is_same_v<decltype(t_src),decltype(t_dest)>)
//         {
//             *t_dest=*t_src;
//         }   
//         else
//         {
//             throw BTreeException(fmt::format("Tried to copy different types"));
//         }
//     }, src, dest);
// }

// // [[nodiscard]] bool RootNode::IsNotNull() const
// // {
// //     return std::visit([](auto&& t_ptr)->bool
// //     {
// //         if(t_ptr)
// //         {
// //             return true;
// //         }
// //         return false;
// //     },ptr);
// // }