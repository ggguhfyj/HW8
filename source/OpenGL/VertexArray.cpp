/**
 * \file
 * \author  JUNSEOK LEE
 * \author JUNSEOK Lee
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */
#include "VertexArray.hpp"
#include "GL.hpp"
#include <GL/glew.h>
#include <GL/gl.h> 
#include "Engine/Engine.hpp"
#include "Engine/Logger.hpp"
namespace OpenGL
{
    /**
     * \brief Creates and configures a Vertex Array Object (VAO) with multiple vertex buffers and optional index buffer
     *
     * This function sets up a complete VAO by:
     * 1. Generating a new VAO handle
     * 2. Binding vertex buffers and configuring their attribute layouts
     * 3. Setting up vertex attribute pointers for each attribute (position, color, texture coords, etc.)
     * 4. Handling both regular floating-point and integer vertex attributes
     * 5. Configuring instanced rendering divisors if needed
     * 6. Optionally binding an index buffer for indexed drawing
     *
     * A VAO encapsulates all the vertex attribute state, so once created, you can simply bind
     * the VAO to use all the configured vertex buffers and their layouts for rendering.
     *
     * \param vertices An initializer list of VertexBuffer objects, each containing:
     *                 - buffer_handle: The OpenGL buffer handle containing vertex data
     *                 - buffer_layout: Description of how the data is organized (attributes, stride, offset)
     * \param index_buffer Optional index buffer handle for indexed rendering (0 if not used)
     *
     * \return VertexArrayHandle The OpenGL handle to the created VAO
     *
     * \note The VAO will be unbound (set to 0) before returning to avoid affecting subsequent OpenGL state
     * \note Each vertex attribute will be assigned sequential attribute indices starting from 0
     */
    VertexArrayHandle CreateVertexArrayObject(std::initializer_list<VertexBuffer> vertices, BufferHandle index_buffer)
    {
        // PSEUDO CODE for CreateVertexArrayObject:
        // 1. Create a new Vertex Array Object (VAO)
        // 2. Bind the VAO to make it active
        // 3. For each vertex buffer:
        //    a. Bind the buffer as GL_ARRAY_BUFFER
        //    b. Calculate the stride (total bytes per vertex)
        //    c. For each attribute in the buffer layout:
        //       - Enable the vertex attribute array
        //       - Set up the vertex attribute pointer (regular or integer)
        //       - Set the vertex attribute divisor for instancing
        // 4. If an index buffer is provided, bind it as GL_ELEMENT_ARRAY_BUFFER
        // 5. Unbind the VAO (bind 0)
        // 6. Return the VAO handle

        // Declare a variable to hold the VAO handle
        VertexArrayHandle vao{};
        GL::GenVertexArrays(1, &vao);
        // Pass 1 for count and the address of vao to store the generated handle
        // Documentation: https://docs.gl/es3/glGenVertexArrays
        GL::BindVertexArray(vao);
        // This makes all subsequent vertex attribute calls affect this VAO
        // Documentation: https://docs.gl/es3/glBindVertexArray

        // Keep track of the current attribute index (starts at 0)
        GLuint attribute_index = 0;

        for(const auto& [buffer_handle, buffer_layout] : vertices)
        // Each VertexBuffer contains a buffer_handle and buffer_layout
        // Use structured binding: for (const auto& [buffer_handle, buffer_layout] : vertices)
        {
            GL::BindBuffer(GL_ARRAY_BUFFER, buffer_handle);// buffer handle is the vertices that we stored in a vertices buffer earlier 
            // This tells OpenGL which buffer to read vertex data from
            // Documentation: https://docs.gl/es3/glBindBuffer

            // Loop through all attributes in buffer_layout.Attributes
            // Sum up each attr_type.SizeBytes to get the total stride
            GLsizei stride = 0;
            for(const auto& attr : buffer_layout.Attributes){
                stride += attr.SizeBytes; //we stored a FLoat2 and a Float3 inside the layout buffer in the constructor.
            }
            Engine::GetLogger().LogEvent( "stride is " + std::to_string(stride));

            // Cast buffer_layout.BufferStartingByteOffset to GLintptr
            GLintptr offset = 0;
            offset = static_cast<GLintptr>(buffer_layout.BufferStartingByteOffset);

            for (Attribute::Type attr_type : buffer_layout.Attributes)
            {
                
                if(attr_type == Attribute::None){
                    continue;
                }
                //enables the attribute for the vertex. 
                GL::EnableVertexAttribArray(attribute_index);


                // Documentation: https://docs.gl/es3/glEnableVertexAttribArray

                 const GLenum    gl_type         = attr_type.GLType;
                 const GLint     component_count = attr_type.ComponentCount;
                 const GLboolean normalized      = attr_type.Normalize;
                 const bool      is_integer      = attr_type.IntAttribute;
                 const GLuint    divisor         = attr_type.Divisor;

                if(is_integer == true){
                    GL::VertexAttribIPointer(attribute_index, component_count, gl_type, stride, reinterpret_cast<const void*>(offset));
                } else{
                    GL::VertexAttribPointer(attribute_index, component_count, gl_type, normalized, stride, reinterpret_cast<const void*>(offset));
                }
                // If true:
                //   Use GL::VertexAttribIPointer for integer attributes
                //   Parameters: (attribute_index, component_count, gl_type, stride, offset as GLvoid*)
                //   Documentation: https://docs.gl/es3/glVertexAttribPointer (contains VertexAttribIPointer)
                // Else:
                //   Use GL::VertexAttribPointer for float/normalized attributes
                //   Parameters: (attribute_index, component_count, gl_type, normalized, stride, offset as GLvoid*)
                //   Documentation: https://docs.gl/es3/glVertexAttribPointer
                GL::VertexAttribDivisor(attribute_index, divisor);
                // Parameters: (attribute_index, divisor)
                // Documentation: https://docs.gl/es3/glVertexAttribDivisor

                attribute_index++;
                offset += attr_type.SizeBytes;
            }
        }
        if(index_buffer != 0){
            GL::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        }
        // If true:
        //   Bind the index buffer as GL_ELEMENT_ARRAY_BUFFER
        //   Documentation: https://docs.gl/es3/glBindBuffer
        GL::BindVertexArray(0);
        // This ensures we don't accidentally modify this VAO later
        // Documentation: https://docs.gl/es3/glBindVertexArray

        return vao;
    }

    VertexArrayHandle CreateVertexArrayObject(VertexBuffer vertices, BufferHandle index_buffer)
    {
        return CreateVertexArrayObject({ vertices }, index_buffer);
    }

}
