export default [
  {
    id: 'channel',
    name: 'Channel variables',
    description: 'streamed continuously',
    groups: [
      {
        id: 'truck',
        name: 'Truck',
        variables: [
          {
            id: 'speed',
            type: 5
          },
          {
            id: 'blinker.left',
            type: 1
          }
        ]
      },
      {
        id: 'trailer',
        name: 'Trailer',
        variables: [
          {
            id: 'connected',
            type: 1
          },
          {
            id: 'wheel.speed',
            type: 5
          }
        ]
      }
    ]
  },
  {
    id: 'config',
    name: 'Config variables',
    description: 'sent only, when a group\'s variable changes',
    groups: [
      {
        id: 'truck',
        name: 'Truck',
        variables: [
          {
            id: 'id',
            type: 15
          },
          {
            id: 'registration.plate',
            type: 15
          }
        ]
      }
    ]
  },
  {
    id: 'event',
    name: 'Event variables',
    description: 'sent only, when an event occurs',
    groups: []
  }
]
