/**
 * \file
 * \author  JUNSEOK LEE
 * \author  JUNSEOK
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */
#include "Buffer.hpp"

#include "GL.hpp"


/*
* span is used because the you can use it for 
* both the static and dynamic(changing the vertex values and drawing)
*
*
*
*/



namespace OpenGL
{
    BufferHandle CreateBuffer(BufferType type, GLsizeiptr size_in_bytes) noexcept
    {
        
        GLuint buffer; //stores the names of the created buffers is a handle
        // https://docs.gl/es3/glGenBuffersr

        GL::GenBuffers(1,&buffer); //whenever you give gen buffers a handle that is already in use it just overwrites the handle but does not de allocate the data,
                                  // which effectively removes the reference to the object.
        
        // https://docs.gl/es3/glBindBuffer
        GL::BindBuffer(static_cast<GLenum>(type), buffer);
        // https://docs.gl/es3/glBufferData
        GL::BufferData(static_cast<GLenum>(type),size_in_bytes,NULL,GL_DYNAMIC_DRAW);

        GL::BindBuffer(static_cast<GLenum>(type), 0);


        return buffer;
    }

    BufferHandle CreateBuffer(BufferType type, std::span<const std::byte> static_buffer_data) noexcept //this is the dynamic one
    {
        
        GLuint buffer;
        GL::GenBuffers(1,&buffer);
        // https://docs.gl/es3/glBindBuffer
        GL::BindBuffer(static_cast<GLenum>(type),buffer);
        // https://docs.gl/es3/glBufferData glBufferData — creates and initializes a buffer object's data store

        //What is a buffer objects.data_store?? -> data store is the actual contiguous block of memory on the GPU where the raw data resides.
        GL::BufferData(
        static_cast<GLenum>(type),
        static_cast<GLsizeiptr>(static_buffer_data.size_bytes()), // correct size
        static_buffer_data.data(),
        GL_STATIC_DRAW
        );

        GL::BindBuffer(static_cast<GLenum>(type), 0);

            return buffer;
        }

        void UpdateBufferData(BufferType type, BufferHandle buffer, std::span<const std::byte> data_to_copy, GLsizei starting_offset) noexcept // I guess this will be used for the dynamic buffer
        {
        
            GL::BindBuffer(static_cast<GLenum>(type), buffer);

        
            GL::BufferSubData(
        static_cast<GLenum>(type),
        starting_offset,
        static_cast<GLsizeiptr>(data_to_copy.size_bytes()), // correct size
        data_to_copy.data()
        );

            GL::BindBuffer(static_cast<GLenum>(type),0);

        }
}
