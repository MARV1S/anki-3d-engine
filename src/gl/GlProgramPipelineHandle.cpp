// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlProgramPipelineHandle.h"
#include "anki/gl/GlProgramPipeline.h"
#include "anki/gl/GlHandleDeferredDeleter.h"

namespace anki {

//==============================================================================
GlProgramPipelineHandle::GlProgramPipelineHandle()
{}

//==============================================================================
GlProgramPipelineHandle::~GlProgramPipelineHandle()
{}

//==============================================================================
Error GlProgramPipelineHandle::create(
	GlCommandBufferHandle& commands,
	std::initializer_list<GlProgramHandle> iprogs)
{
	Array<GlProgramHandle, 6> progs;

	U count = 0;
	for(GlProgramHandle prog : iprogs)
	{
		progs[count++] = prog;
	}

	return commonConstructor(commands, &progs[0], &progs[0] + count);
}

//==============================================================================
Error GlProgramPipelineHandle::commonConstructor(
	GlCommandBufferHandle& commands,
	const GlProgramHandle* progsBegin, const GlProgramHandle* progsEnd)
{
	class Command: public GlCommand
	{
	public:
		GlProgramPipelineHandle m_ppline;
		Array<GlProgramHandle, 6> m_progs;
		U8 m_progsCount;

		Command(GlProgramPipelineHandle& ppline, 
			const GlProgramHandle* progsBegin, const GlProgramHandle* progsEnd)
		:	m_ppline(ppline)
		{
			m_progsCount = 0;
			const GlProgramHandle* prog = progsBegin;
			do
			{
				m_progs[m_progsCount++] = *prog;
			} while(++prog != progsEnd);
		}

		void operator()(GlCommandBuffer*)
		{
			Error err = m_ppline._get().create(
				&m_progs[0], &m_progs[0] + m_progsCount);
			ANKI_ASSERT(!err);

			GlHandleState oldState = m_ppline._setState(GlHandleState::CREATED);
			ANKI_ASSERT(oldState == GlHandleState::TO_BE_CREATED);
			(void)oldState;
		}
	};

	using Alloc = GlAllocator<GlProgramPipeline>;
	using DeleteCommand = GlDeleteObjectCommand<GlProgramPipeline, Alloc>;
	using Deleter = 
		GlHandleDeferredDeleter<GlProgramPipeline, Alloc, DeleteCommand>;

	Error err = _createAdvanced(
		&commands._get().getQueue().getDevice(),
		commands._get().getGlobalAllocator(), 
		Deleter());

	if(!err)
	{
		_setState(GlHandleState::TO_BE_CREATED);

		commands._pushBackNewCommand<Command>(*this, progsBegin, progsEnd);
	}

	return err;
}

//==============================================================================
void GlProgramPipelineHandle::bind(GlCommandBufferHandle& commands)
{
	ANKI_ASSERT(isCreated());

	class Command: public GlCommand
	{
	public:
		GlProgramPipelineHandle m_ppline;

		Command(GlProgramPipelineHandle& ppline)
			: m_ppline(ppline)
		{}

		void operator()(GlCommandBuffer* commands)
		{
			GlState& state = commands->getQueue().getState();

			if(state.m_crntPpline != m_ppline._get().getGlName())
			{
				m_ppline._get().bind();

				state.m_crntPpline = m_ppline._get().getGlName();
			}
		}
	};

	commands._pushBackNewCommand<Command>(*this);
}

//==============================================================================
GlProgramHandle GlProgramPipelineHandle::getAttachedProgram(GLenum type) const
{
	ANKI_ASSERT(isCreated());
	serializeOnGetter();
	return _get().getAttachedProgram(type);
}

} // end namespace anki

